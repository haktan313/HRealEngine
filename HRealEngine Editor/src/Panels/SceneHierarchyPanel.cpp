
#include "SceneHierarchyPanel.h"
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "HRealEngine/Asset/AssetManager.h"
#include "HRealEngine/Core/Logger.h"
#include "HRealEngine/Core/MeshLoader.h"
#include "HRealEngine/Renderer/Material.h"
#include "HRealEngine/Scripting/ScriptEngine.h"

namespace HRealEngine
{
    extern const std::filesystem::path g_AssetsDirectory;
    
    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
    {
        m_Context = context;
        m_SelectedEntity = {};
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        auto& registry = m_Context->GetRegistry();
        auto view = registry.view<TransformComponent, EntityNameComponent>();
        for (auto entity : view)
        {
            Entity e{entity, m_Context.get()};
            bool isChild = false;
            if (e.HasComponent<ChildrenManagerComponent>())
                isChild = e.GetComponent<ChildrenManagerComponent>().ParentHandle != 0;
        
            if (!isChild)
                DrawEntityNode(e);
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
            m_SelectedEntity = {};

        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_SelectedEntity = m_Context->CreateEntity("Empty Entity");
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_ENTITY"))
            {
                UUID droppedUUID = *(UUID*)payload->Data;
                Entity droppedEntity = m_Context->GetEntityByUUID(droppedUUID);
                if (droppedEntity)
                    m_Context->RemoveParent(droppedEntity);
            }
            ImGui::EndDragDropTarget();
            
        }
        ImGui::Separator();
        if (m_SelectedEntity)
        {
            ImGui::Begin("Properties");
            if (m_SelectedEntity)
            {
                ImGui::Begin("Properties");
                DrawComponents(m_SelectedEntity);
                ImGui::End();
            }
            ImGui::End();
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
    {
        m_SelectedEntity = entity;
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& Tag = entity.GetComponent<EntityNameComponent>().Name;

        bool hasChildren = false;
        if (entity.HasComponent<ChildrenManagerComponent>())
            hasChildren = !entity.GetComponent<ChildrenManagerComponent>().Children.empty();
        
        ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf;
        
        bool bOpened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, Tag.c_str());

        if (!hasChildren)
        {
            ImVec2 itemMin = ImGui::GetItemRectMin();
            float fontSize = ImGui::GetFontSize();
            float radius = fontSize * 0.15f;
            ImVec2 center = ImVec2(itemMin.x + fontSize * 0.5f, itemMin.y + fontSize * 0.5f + ImGui::GetStyle().FramePadding.y);
            ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, IM_COL32(180, 180, 180, 200));
        }
        
        if (ImGui::IsItemClicked())
            m_SelectedEntity = entity;

        if (ImGui::BeginDragDropSource())
        {
            UUID uuid = entity.GetUUID(); 
            ImGui::SetDragDropPayload("SCENE_ENTITY", &uuid, sizeof(UUID));
            ImGui::Text("%s", Tag.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_ENTITY"))
            {
                UUID droppedUUID = *(UUID*)payload->Data;
                Entity droppedEntity = m_Context->GetEntityByUUID(droppedUUID);
                if (droppedEntity && droppedEntity != entity)
                    m_Context->SetParent(droppedEntity, entity);
            }
            ImGui::EndDragDropTarget();
        }

        bool bDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                bDeleted = true;
            if (ImGui::MenuItem("Create Child Entity"))
            {
                Entity child = m_Context->CreateEntity("Child Entity");
                m_Context->SetParent(child, entity);
            }
            if (entity.HasComponent<ChildrenManagerComponent>() && entity.GetComponent<ChildrenManagerComponent>().ParentHandle != 0)
                if (ImGui::MenuItem("Unparent"))
                    m_Context->RemoveParent(entity);
            ImGui::EndPopup();
        }
        
        if (bOpened)
        {
            if (hasChildren)
            {
                auto children = m_Context->GetChildren(entity);
                for (auto& child : children)
                    DrawEntityNode(child);
            }
            ImGui::TreePop();
        }

        if (bDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_SelectedEntity == entity)
                m_SelectedEntity = {};
        }
    }

    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        
        ImGui::PushID(label.c_str());
        
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, .0f, .0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, .0f, .0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, .0f, .0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        
        ImGui::Columns(1);
        ImGui::PopID();
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4,4});
            float lineHight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
            ImGui::Separator();
            
            bool bOpen = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
            ImGui::PopStyleVar();
            ImGui::SameLine(contentRegionAvailable.x - lineHight * 0.5f);
            if (ImGui::Button("...", ImVec2{lineHight,lineHight}))
                ImGui::OpenPopup("ComponentSettings");
            
            bool bRemoveComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem("Remove Component"))
                    bRemoveComponent = true;
                ImGui::EndPopup();
            }
            
            if (bOpen)
            {
                uiFunction(component);
                ImGui::TreePop();
            }
            
            if (bRemoveComponent)
                entity.RemoveComponent<T>();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<EntityNameComponent>())
        {
            auto& tag = entity.GetComponent<EntityNameComponent>().Name;

            char extraSpace[256];
            memset(extraSpace, 0, sizeof(extraSpace));
            strcpy_s(extraSpace, sizeof(extraSpace), tag.c_str());
            if (ImGui::InputText("##Tag", extraSpace, sizeof(extraSpace)))
            {
                tag = std::string(extraSpace);
            }
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");
        if (ImGui::BeginPopup("AddComponent"))
        {
            ShowAddComponentEntry<TagComponent>("Tag Component");
            ShowAddComponentEntry<LightComponent>("Light Component");
            ShowAddComponentEntry<TextComponent>("Text Component");
            ShowAddComponentEntry<TransformComponent>("Transform Component");
            ShowAddComponentEntry<CameraComponent>("Camera Component");
            ShowAddComponentEntry<ScriptComponent>("Script Component");
            ShowAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
            ShowAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
            ShowAddComponentEntry<BehaviorTreeComponent>("Behavior Tree Component");
            ShowAddComponentEntry<AIControllerComponent>("AI Controller Component");
            ShowAddComponentEntry<PerceivableComponent>("Perceivable Component");
            ShowAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            ShowAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
            ShowAddComponentEntry<Rigidbody3DComponent>("Rigidbody 3D");
            ShowAddComponentEntry<BoxCollider3DComponent>("Box Collider 3D");
            ShowAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
            ShowAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        DrawComponent<TagComponent>("Tag", entity, [](auto& component)
        {
            ImGui::Text("Tags");
            ImGui::Separator();

            for (int i = 0; i < (int)component.Tags.size(); i++)
            {
                ImGui::PushID(i);

                ImGui::TextUnformatted(component.Tags[i].c_str());
                ImGui::SameLine();

                if (ImGui::SmallButton("X"))
                {
                    component.Tags.erase(component.Tags.begin() + i);
                    ImGui::PopID();
                    break;
                }

                ImGui::PopID();
            }

            ImGui::Spacing();
            ImGui::Separator();

            static std::unordered_map<uint64_t, std::string> s_NewTagBuffers;
            uint64_t key = (uint64_t)(uintptr_t)&component;
            std::string& newTag = s_NewTagBuffers[key];

            ImGui::InputText("##NewTag", &newTag);
            ImGui::SameLine();

            if (ImGui::Button("Add"))
            {
                if (!newTag.empty())
                {
                    bool exists = std::find(component.Tags.begin(), component.Tags.end(), newTag) != component.Tags.end();
                    if (!exists)
                        component.Tags.push_back(newTag);

                    newTag.clear();
                }
            }
        });
        DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
        {
            DrawVec3Control("Position", component.Position);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            DrawVec3Control("Rotation", rotation);
            component.Rotation = glm::radians(rotation);
            DrawVec3Control("Scale", component.Scale, 1.0f);
        });
        DrawComponent<LightComponent>("Light Component", entity, [](auto& component)
        {
            const char* types[] = { "Directional", "Point", "Spot" };
            int type = (int)component.Type;
            if (ImGui::Combo("Type", &type, types, 3))
                component.Type = (LightComponent::LightType)type;

            ImGui::ColorEdit3("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Intensity", &component.Intensity, 0.1f, 0.0f, 100.0f);

            if (component.Type == LightComponent::LightType::Directional || component.Type == LightComponent::LightType::Spot)
                ImGui::DragFloat3("Direction", glm::value_ptr(component.Direction), 0.01f, -1.0f, 1.0f);

            if (component.Type == LightComponent::LightType::Point || component.Type == LightComponent::LightType::Spot)
                ImGui::DragFloat("Radius", &component.Radius, 0.1f, 0.0f, 1000.0f);

            ImGui::Checkbox("Cast Shadows", &component.CastShadows);
        });
        DrawComponent<TextComponent>("Text Component", entity, [](auto& component)
        {
            ImGui::InputTextMultiline("Text String", &component.TextString);
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Kerning", &component.Kerning, 0.1f, -50.0f, 50.0f);
            ImGui::DragFloat("Line Spacing", &component.LineSpacing, 0.1f, -50.0f, 50.0f);
        });
        DrawComponent<CameraComponent>("Camera",entity, [](auto& component)
        {
            auto& camera = component.Camera;

            ImGui::Checkbox("Primary", &component.PrimaryCamera);
            
            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
            
            if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
            {
                for (int i = 0; i < 2; i++)
                {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                    {
                        currentProjectionTypeString = projectionTypeStrings[i];
                        camera.SetProjectionType((SceneCamera::ProjectionType)i);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthographicSize();
                if (ImGui::DragFloat("Size", &orthoSize))
                    camera.SetOrthographicSize(orthoSize);
                
                float orthoNear = camera.GetNearClip();
                if (ImGui::DragFloat("Near", &orthoNear))
                    camera.SetNearClip(orthoNear);
                
                float orthoFar = camera.GetFarClip();
                if (ImGui::DragFloat("Far", &orthoFar))
                    camera.SetFarClip(orthoFar);
                ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
            }
            else
            {
                float perspectiveFOV = glm::degrees(camera.GetPerspectiveFOV());
                if (ImGui::DragFloat("FOV", &perspectiveFOV))
                    camera.SetPerspectiveFOV(glm::radians(perspectiveFOV));
                
                float perspectiveNear = camera.GetPerspectiveNear();
                if (ImGui::DragFloat("Near", &perspectiveNear))
                    camera.SetPerspectiveNear(perspectiveNear);
                
                float perspectiveFar = camera.GetPerspectiveFar();
                if (ImGui::DragFloat("Far", &perspectiveFar))
                    camera.SetPerspectiveFar(perspectiveFar);
            }
        });
        DrawComponent<ScriptComponent>("Script Component", entity, [entity, scene = m_Context](auto& component) mutable 
        {
            bool bScriptClassIsExist = ScriptEngine::IsEntityClassExist(component.ClassName);
            
            static char className[64];
            strcpy_s(className, sizeof(className), component.ClassName.c_str());
            
            if (!bScriptClassIsExist)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 0.2f, 0.2f, 1.0f});
            
            if (ImGui::InputText("Class", className, sizeof(className)))
                component.ClassName = className;
            
            /*if (!bScriptClassIsExist)
                ImGui::PopStyleColor();*/
            bool bIsSceneRunning = scene->IsRunning();
            if (bIsSceneRunning)
            {
                Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntitySriptInstance(entity.GetUUID());
                if (scriptInstance)
                {
                    const auto& fields = scriptInstance->GetScriptClass()->GetFields();
                    for (const auto& [name, field] : fields)
                    {
                        if (field.Type == ScriptFieldType::Float)
                        {
                            float data = scriptInstance->GetFieldValue<float>(name);
                            if (ImGui::DragFloat(name.c_str(), &data))
                                scriptInstance->SetFieldValue(name, data);
                        }
                    }
                }
            }
            else
            {
                if (bScriptClassIsExist)
                {
                    Ref<ScriptClass> scriptClass = ScriptEngine::GetEntityClass(component.ClassName);
                    const auto& fields = scriptClass->GetFields();
                    auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
                    for (const auto& [name, field] : fields)
                    {
                        if (entityFields.find(name) != entityFields.end())
                        {
                            ScriptFieldInstance& scriptField = entityFields.at(name);
                            if (field.Type == ScriptFieldType::Float)
                            {
                                float data = scriptField.GetValue<float>();
                                if (ImGui::DragFloat(name.c_str(), &data))
                                    scriptField.SetValue(data);
                            }
                            if (field.Type == ScriptFieldType::Double)
                            {
                                double data = scriptField.GetValue<double>();
                                float floatData = (float)data;
                                if (ImGui::DragFloat(name.c_str(), &floatData))
                                {
                                    data = (double)floatData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Bool)
                            {
                                bool data = scriptField.GetValue<bool>();
                                if (ImGui::Checkbox(name.c_str(), &data))
                                    scriptField.SetValue(data);
                            }
                            if (field.Type == ScriptFieldType::Char)
                            {
                                char data = scriptField.GetValue<char>();
                                if (ImGui::InputText(name.c_str(), &data, 2))
                                    scriptField.SetValue(data);
                            }
                            if (field.Type == ScriptFieldType::Byte)
                            {
                                int8_t data = scriptField.GetValue<int8_t>();
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, INT8_MIN, INT8_MAX))
                                {
                                    data = (int8_t)intData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Short)
                            {
                                int16_t data = scriptField.GetValue<int16_t>();
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, INT16_MIN, INT16_MAX))
                                {
                                    data = (int16_t)intData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Int)
                            {
                                int data = scriptField.GetValue<int>();
                                if (ImGui::DragInt(name.c_str(), &data))
                                    scriptField.SetValue(data);
                            }
                            if (field.Type == ScriptFieldType::Long)
                            {
                                int64_t data = scriptField.GetValue<int64_t>();
                                long longData = (long)data;
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &longData))
                                {
                                    data = (int64_t)longData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::UByte)
                            {
                                uint8_t data = scriptField.GetValue<uint8_t>();
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, 0, UINT8_MAX))
                                {
                                    data = (uint8_t)intData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::UShort)
                            {
                                uint16_t data = scriptField.GetValue<uint16_t>();
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, 0, UINT16_MAX))
                                {
                                    data = (uint16_t)intData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::UInt)
                            {
                                uint32_t data = scriptField.GetValue<uint32_t>();
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, 0, INT32_MAX))
                                {
                                    data = (uint32_t)intData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::ULong)
                            {
                                uint64_t data = scriptField.GetValue<uint64_t>();
                                long longData = (long)data;
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &longData))
                                {
                                    data = (uint64_t)longData;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Vector2)
                            {
                                glm::vec2 data = scriptField.GetValue<glm::vec2>();
                                if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
                                    scriptField.SetValue(data);
                            } 
                            if (field.Type == ScriptFieldType::Vector3)
                            {
                                glm::vec3 data = scriptField.GetValue<glm::vec3>();
                                if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
                                    scriptField.SetValue(data);
                            }
                            if (field.Type == ScriptFieldType::Vector4)
                            {
                                glm::vec4 data = scriptField.GetValue<glm::vec4>();
                                if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
                                    scriptField.SetValue(data);
                            }
                            if (field.Type == ScriptFieldType::Entity)
                            {

                            }
                            if (field.Type == ScriptFieldType::String)
                            {
                                std::string value = scriptField.GetValue<std::string>();
                                char buf[256] = {};
                                strncpy_s(buf, sizeof(buf), value.c_str(), _TRUNCATE);
                                if (ImGui::InputText(name.c_str(), buf, sizeof(buf)))
                                    scriptField.SetValue<std::string>(std::string(buf));
                            }
                        }
                        else
                        {
                            if (field.Type == ScriptFieldType::Float)
                            {
                                float data = 0.0f;
                                if (ImGui::DragFloat(name.c_str(), &data))
                                {
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Double)
                            {
                                double data = 0.0;
                                float floatData = (float)data;
                                if (ImGui::DragFloat(name.c_str(), &floatData))
                                {
                                    data = (double)floatData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Bool)
                            {
                                bool data = false;
                                if (ImGui::Checkbox(name.c_str(), &data))
                                {
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Char)
                            {
                                char data = '\0';
                                if (ImGui::InputText(name.c_str(), &data, 2))
                                {
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Byte)
                            {
                                int8_t data = 0;
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, INT8_MIN, INT8_MAX))
                                {
                                    data = (int8_t)intData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Short)
                            {
                                int16_t data = 0;
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, INT16_MIN, INT16_MAX))
                                {
                                    data = (int16_t)intData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Int)
                            {
                                int data = 0;
                                if (ImGui::DragInt(name.c_str(), &data))
                                {
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Long)
                            {
                                int64_t data = 0;
                                long longData = (long)data;
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &longData))
                                {
                                    data = (int64_t)longData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::UByte)
                            {
                                uint8_t data = 0;
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, 0, UINT8_MAX))
                                {
                                    data = (uint8_t)intData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::UShort)
                            {
                                uint16_t data = 0;
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, 0, UINT16_MAX))
                                {
                                    data = (uint16_t)intData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::UInt)
                            {
                                uint32_t data = 0;
                                int intData = (int)data;
                                if (ImGui::DragInt(name.c_str(), &intData, 1.0f, 0, INT32_MAX))
                                {
                                    data = (uint32_t)intData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::ULong)
                            {
                                uint64_t data = 0;
                                long longData = (long)data;
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &longData))
                                {
                                    data = (uint64_t)longData;
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Vector2)
                            {
                                glm::vec2 data = glm::vec2(0.0f);
                                if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
                                {
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Vector3)
                            {
                                glm::vec3 data = glm::vec3(0.0f);
                                if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
                                {
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Vector4)
                            {
                                glm::vec4 data = glm::vec4(0.0f);
                                if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
                                {
                                    ScriptFieldInstance& scriptField = entityFields[name];
                                    scriptField.Field = field;
                                    scriptField.SetValue(data);
                                }
                            }
                            if (field.Type == ScriptFieldType::Entity)
                            {

                            }
                            if (field.Type == ScriptFieldType::String)
                            {
                                char buf[256] = {};
                                if (ImGui::InputText(name.c_str(), buf, sizeof(buf)))
                                {
                                    ScriptFieldInstance& s = entityFields[name];
                                    s.Field = field;
                                    s.SetValue<std::string>(std::string(buf));
                                }
                            }
                        }
                    }
                }
            }
            if (!bScriptClassIsExist)
                ImGui::PopStyleColor();
        });
        DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
        {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            //ImGui::Button("Texture");
            std::string label = "None";
            bool isTextureValid = false;
            if (component.Texture != 0)
            {
                if (AssetManager::IsAssetHandleValid(component.Texture)
                    && AssetManager::GetAssetType(component.Texture) == AssetType::Texture)
                {
                    const AssetMetadata& metadata = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(component.Texture);
                    label = metadata.FilePath.filename().string();
                    isTextureValid = true;
                }
                else
                {
                    label = "Invalid";
                }
            }

            ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
            buttonLabelSize.x += 20.0f;
            float buttonLabelWidth = glm::max<float>(100.0f, buttonLabelSize.x);

            ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    /*const wchar_t* path = (const wchar_t*)payload->Data;
                    //std::filesystem::path fullPath = std::filesystem::path(g_AssetsDirectory) / path;
                    std::filesystem::path fullPath(path);
                    //component.Texture = Texture2D::Create(fullPath.string());
                    Ref<Texture2D> texture = Texture2D::Create(fullPath.string());
                    if (texture->IsLoaded())
                        component.Texture = texture->Handle;//texture;*/
                    AssetHandle handle = *(AssetHandle*)payload->Data;
                    if (AssetManager::GetAssetType(handle) == AssetType::Texture)
                    {
                        component.Texture = handle;
                    }
                    else
                    {
                        LOG_CORE_WARN("Dropped asset is not a texture.");
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (isTextureValid)
            {
                ImGui::SameLine();
                ImVec2 xLabelSize = ImGui::CalcTextSize("X");
                float buttonSize = xLabelSize.y + ImGui::GetStyle().FramePadding.y * 2.0f;
                if (ImGui::Button("X", ImVec2(buttonSize, buttonSize)))
                {
                    component.Texture = 0;
                }
            }

            ImGui::SameLine();
            ImGui::Text("Texture");
            
            ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
            ImGui::DragInt("Order In Layer", &component.OrderInLayer, 1, 0, 100);
        });
        DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component)
        {
            ImGui::DragFloat3("Pivot Offset", glm::value_ptr(component.PivotOffset), 0.1f);
            ImGui::Spacing();
            ImGui::Text("Mesh");
            ImGui::Button("Drop .hmesh here", ImVec2(200, 0));
        
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    /*const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path droppedPath = path;
        
                    if (droppedPath.extension() == ".hmesh")
                    {
                        auto shader = Shader::Create("assets/shaders/StaticMesh.glsl");
        
                        component.Mesh = ObjLoader::GetOrLoad(droppedPath, "assets", shader);
                        component.MeshAssetPath = droppedPath;
                        if (component.Mesh)
                        {
                            const size_t slotCount = component.Mesh->MaterialPaths.size();
                            component.MaterialOverrides.clear();
                            component.MaterialOverrides.resize(slotCount);
                            LOG_CORE_INFO("Mesh material slots: {}", slotCount);
                        }
                        LOG_CORE_INFO("Assigned mesh: {}", droppedPath.string());
                    }*/
                    AssetHandle handle = *(AssetHandle*)payload->Data;
                    if (AssetManager::GetAssetType(handle) == AssetType::Mesh)
                    {
                        component.Mesh = handle;
                        auto shader = Shader::Create("assets/shaders/StaticMesh.glsl");
                        auto& metaData = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(handle);
                        std::filesystem::path droppedPath = metaData.FilePath;
                        component.MeshAssetPath = droppedPath;
                        
                        auto meshGPU = MeshLoader::GetOrLoad(droppedPath, "assets", shader);
                        if (meshGPU)
                        {
                            //const size_t slotCount = meshGPU->MaterialPaths.size();
                            const size_t slotCount = meshGPU->MaterialHandles.size();
                            /*component.MaterialOverrides.clear();
                            component.MaterialOverrides.resize(slotCount);*/
                            component.MaterialHandleOverrides.clear();
                            component.MaterialHandleOverrides.resize(slotCount);
                            LOG_CORE_INFO("Mesh material slots: {}", slotCount);
                            for (size_t i = 0; i < slotCount; i++)
                                component.MaterialHandleOverrides[i] = meshGPU->MaterialHandles[i];

                            auto meshGPUAsset = AssetManager::GetAsset<MeshGPU>(handle);
                            if (meshGPUAsset)
                            {
                                meshGPUAsset->Shader = shader;
                                meshGPUAsset->VAO = meshGPU->VAO;
                                meshGPUAsset->IndexCount = meshGPU->IndexCount;
                                //meshGPUAsset->MaterialPaths = meshGPU->MaterialPaths;
                                meshGPUAsset->MaterialHandles = meshGPU->MaterialHandles;
                                meshGPUAsset->Submeshes = meshGPU->Submeshes;
                            }
                        }
                    }
                    else
                    {
                        LOG_CORE_WARN("Dropped asset is not a mesh.");
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            
            ImGui::Button("Texture");
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    /*const wchar_t* path = (const wchar_t*)payload->Data;
                    //std::filesystem::path fullPath = std::filesystem::path(g_AssetsDirectory) / path;
                    std::filesystem::path fullPath(path);
                    Ref<Texture2D> texture = Texture2D::Create(fullPath.string());
                    if (texture->IsLoaded())
                        component.Texture = texture;*/
                    AssetHandle handle = *(AssetHandle*)payload->Data;
                    if (AssetManager::GetAssetType(handle) == AssetType::Texture)
                    {
                        component.Texture = handle;
                    }
                    else
                    {
                        LOG_CORE_WARN("Dropped asset is not a texture.");
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (component.Texture)
            {
                ImGui::SameLine();
                ImGui::Text("Loaded");
            }
            ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);

            if (component.Mesh)
            {
                auto meshGPU = AssetManager::GetAsset<MeshGPU>(component.Mesh);
                if (!meshGPU)
                {
                    LOG_CORE_WARN("MeshRendererComponent: Mesh asset is invalid.");
                    return;
                }
                const size_t slotCount = meshGPU->MaterialHandles.size();
                if (slotCount > 0)
                {
                    ImGui::Separator();
                    ImGui::Text("Materials");           

                    /*if (component.MaterialOverrides.size() != slotCount)
                        component.MaterialOverrides.resize(slotCount);*/
                    if (component.MaterialHandleOverrides.size() != slotCount)
                        component.MaterialHandleOverrides.resize(slotCount, 0);

                    for (size_t i = 0; i < slotCount; i++)
                    {
                        ImGui::PushID((int)i);          
                        /*const std::string& defaultPath = meshGPU->MaterialPaths[i];          
                        const std::string& overridePath = component.MaterialOverrides[i];*/
                        AssetHandle defaultHandle  = (i < meshGPU->MaterialHandles.size()) ? meshGPU->MaterialHandles[i] : 0;
                        AssetHandle overrideHandle = (i < component.MaterialHandleOverrides.size()) ? component.MaterialHandleOverrides[i] : 0;
                        //const bool hasOverride = !overridePath.empty();
                        const bool hasOverride = overrideHandle != 0;

                        ImGui::Text("Slot %d", (int)i);
                        ImGui::SameLine();          

                        std::string buttonLabel = hasOverride ? "Override (.hmat)" : "Drop .hmat";
                        ImGui::Button(buttonLabel.c_str(), ImVec2(200, 0));         

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                /*const wchar_t* path = (const wchar_t*)payload->Data;
                                std::filesystem::path dropped = path;           
                                if (dropped.extension() == ".hmat")
                                {
                                    component.MaterialOverrides[i] = dropped.generic_string();
                                    LOG_CORE_INFO("Material override set: slot={} -> {}", i, component.MaterialOverrides[i]);
                                }*/
                                AssetHandle h = *(AssetHandle*)payload->Data;
                                if (AssetManager::GetAssetType(h) == AssetType::Material)
                                {
                                    if (component.MaterialHandleOverrides.size() != slotCount)
                                        component.MaterialHandleOverrides.resize(slotCount, 0);
                                
                                    component.MaterialHandleOverrides[i] = h;
                                
                                    /*const auto& md = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(h);
                                    component.MaterialOverrides[i] = md.FilePath.generic_string();*/
                                }
                                else
                                {
                                    LOG_CORE_WARN("Dropped asset is not a material.");
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }           

                        ImGui::SameLine();
                        if (ImGui::SmallButton("Clear"))
                        {
                            /*component.MaterialHandleOverrides[i] = 0;
                            component.MaterialOverrides[i].clear();
                            //component.MaterialOverrides[i].clear();*/
                            component.MaterialHandleOverrides[i] = 0;

                        }

                        /*ImGui::TextDisabled("Default: %s", defaultPath.empty() ? "(empty)" : defaultPath.c_str());
                        if (hasOverride)
                            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Using Override: %s", overridePath.c_str());
                        else
                            ImGui::TextDisabled("Using Default");           
                        ImGui::Separator();
                        ImGui::PopID();*/
                        AssetHandle activeMatHandle = (overrideHandle != 0) ? overrideHandle : defaultHandle;

                        if (activeMatHandle != 0 && AssetManager::IsAssetHandleValid(activeMatHandle))
                        {
                            Ref<HMaterial> mat = AssetManager::GetAsset<HMaterial>(activeMatHandle);
                            if (mat)
                            {
                                if (ImGui::TreeNodeEx("Material Properties", ImGuiTreeNodeFlags_DefaultOpen))
                                {
                                    ImGui::ColorEdit4("Base Color", glm::value_ptr(mat->Color));
                                    ImGui::SliderFloat("Shininess", &mat->Shininess, 1.0f, 256.0f);

                                    auto TextureDropButton = [&](const char* label, AssetHandle& texHandle)
                                    {
                                        ImGui::Text("%s: %llu", label, (uint64_t)texHandle);
                                        ImGui::SameLine();
                                        ImGui::Button(("Drop " + std::string(label)).c_str(), ImVec2(160, 0));

                                        if (ImGui::BeginDragDropTarget())
                                        {
                                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                                            {
                                                AssetHandle h = *(AssetHandle*)payload->Data;
                                                if (AssetManager::GetAssetType(h) == AssetType::Texture)
                                                {
                                                    texHandle = h;
                                                    
                                                    if (std::string(label) == "Albedo")
                                                        mat->AlbedoTextureCache = nullptr;
                                                    if (std::string(label) == "Specular")
                                                        mat->SpecularTextureCache = nullptr;
                                                    if (std::string(label) == "Normal")
                                                        mat->NormalTextureCache = nullptr;
                                                       
                                                } 
                                                else
                                                {
                                                    LOG_CORE_WARN("Dropped asset is not a texture.");
                                                }
                                            }
                                            ImGui::EndDragDropTarget();
                                        }

                                        ImGui::SameLine();
                                        if (ImGui::SmallButton(("Clear##" + std::string(label)).c_str()))
                                        {
                                            texHandle = 0;
                                            if (std::string(label) == "Albedo")
                                                mat->AlbedoTextureCache = nullptr;
                                            if (std::string(label) == "Specular")
                                                mat->SpecularTextureCache = nullptr;
                                            if (std::string(label) == "Normal")
                                                mat->NormalTextureCache = nullptr;
                                        }
                                    };

                                    TextureDropButton("Albedo",   mat->AlbedoTextureHandle);
                                    TextureDropButton("Specular", mat->SpecularTextureHandle);
                                    TextureDropButton("Normal",   mat->NormalTextureHandle);

                                    ImGui::TreePop();
                                }
                            }
                        }

                        auto GetLabel = [](AssetHandle h) -> std::string
                        {
                            if (h == 0)
                                return "(none)";
                            if (!AssetManager::IsAssetHandleValid(h))
                                return "(invalid)";
                            auto& md = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(h);
                            return md.FilePath.filename().string();
                        };
                        
                        std::string defaultLabel  = GetLabel(defaultHandle);
                        std::string overrideLabel = GetLabel(overrideHandle);
                        
                        ImGui::TextDisabled("Default: %s", defaultLabel.c_str());
                        if (hasOverride)
                            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "Using Override: %s", overrideLabel.c_str());
                        else
                            ImGui::TextDisabled("Using Default");
                        ImGui::Separator();
                        ImGui::PopID();
                    }
                }
                else
                    ImGui::TextDisabled("No material slots found in this mesh.");
            }
            else
                ImGui::TextDisabled("Assign a .hmesh to see material slots.");
        });
        DrawComponent<BehaviorTreeComponent>("Behavior Tree Component", entity, [](auto& component)
        {
            ImGui::Text("Behavior Tree");
            std::string buttonLabel = "Drop .hbtree here";
            if (component.BehaviorTreeAsset)
            {
                const auto& metadata = Project::GetActive()->GetEditorAssetManager()->GetAssetMetadata(component.BehaviorTreeAsset);
                buttonLabel = metadata.FilePath.filename().string();
            }

            ImGui::Button(buttonLabel.c_str(), ImVec2(200, 0));
            /*ImGui::Button("Drop .btree here", ImVec2(200, 0));*/
                
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    AssetHandle handle = *(AssetHandle*)payload->Data;
                    if (AssetManager::GetAssetType(handle) == AssetType::BehaviorTree)
                    {
                        component.BehaviorTreeAsset = handle;
                        LOG_CORE_INFO("Assigned behavior tree asset handle: {}", (uint64_t)handle);
                    }
                    else
                    {
                        LOG_CORE_WARN("Dropped asset is not a behavior tree.");
                    }
                }
                ImGui::EndDragDropTarget();
            }
            if (component.BehaviorTreeAsset)
            {
                ImGui::SameLine();
                ImGui::Text("Loaded");
            }
        });
        DrawComponent<AIControllerComponent>("AI Controller Component", entity, [](auto& component)
        {
            ImGui::Text("AI Controller");
            ImGui::DragFloat("Update Interval", &component.UpdateInterval, 0.01f, 0.05f, 2.0f, "%.2f s");
            
            ImGui::Separator();
            
            ImGui::Checkbox("Sight Enabled", &component.EnabledPerceptions[PercaptionType::Sight]);
            if (component.EnabledPerceptions[PercaptionType::Sight])
            {
                ImGui::Indent();
                ImGui::DragFloat("Sight Radius", &component.SightSettings.SightRadius, 0.5f, 1.0f, 200.0f);
                ImGui::DragFloat("Field of View", &component.SightSettings.FieldOfView, 1.0f, 10.0f, 360.0f, "%.0f deg");
                ImGui::DragFloat("Forget Duration##Sight", &component.SightSettings.ForgetDuration, 0.1f, 0.0f, 30.0f, "%.1f s");
                
                if (ImGui::TreeNode("Detectable Types##Sight"))
                {
                    const char* typeNames[] = { "Player", "Enemy", "Neutral", "Environment" };
                    for (int i = 0; i < 4; i++)
                    {
                        auto it = std::find(component.SightSettings.DetectableTypes.begin(),
                            component.SightSettings.DetectableTypes.end(), (PerceivableType)i);
                        bool found = it != component.SightSettings.DetectableTypes.end();
                        if (ImGui::Checkbox(typeNames[i], &found))
                        {
                            if (found)
                                component.SightSettings.DetectableTypes.push_back((PerceivableType)i);
                            else
                                component.SightSettings.DetectableTypes.erase(it);
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::Unindent();
            }
            
            ImGui::Separator();
            
            ImGui::Checkbox("Hearing Enabled", &component.EnabledPerceptions[PercaptionType::Hearing]);
            if (component.EnabledPerceptions[PercaptionType::Hearing])
            {
                ImGui::Indent();
                ImGui::DragFloat("Hearing Radius", &component.HearingSettings.HearingRadius, 0.5f, 1.0f, 200.0f);
                ImGui::DragFloat("Forget Duration##Hearing", &component.HearingSettings.ForgetDuration, 0.1f, 0.0f, 30.0f, "%.1f s");
                
                if (ImGui::TreeNode("Detectable Types##Hearing"))
                {
                    const char* typeNames[] = { "Player", "Enemy", "Neutral", "Environment" };
                    for (int i = 0; i < 4; i++)
                    {
                        auto it = std::find(component.HearingSettings.DetectableTypes.begin(),
                            component.HearingSettings.DetectableTypes.end(), (PerceivableType)i);
                        bool found = it != component.HearingSettings.DetectableTypes.end();
                        if (ImGui::Checkbox(typeNames[i], &found))
                        {
                            if (found)
                                component.HearingSettings.DetectableTypes.push_back((PerceivableType)i);
                            else
                                component.HearingSettings.DetectableTypes.erase(it);
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::Unindent();
            }
            
            ImGui::Separator();
            
            ImGui::TextDisabled("Current Perceptions: %d", (int)component.CurrentPerceptions.size());
            ImGui::TextDisabled("Previous Perceptions: %d", (int)component.PreviousPerceptions.size());
            ImGui::TextDisabled("Forgotten: %d", (int)component.ForgottenPerceptions.size());
        });
        DrawComponent<PerceivableComponent>("Perceivable Component", entity, [](auto& component)
        {
            ImGui::Checkbox("Is Detectable", &component.bIsDetectable);
            ImGui::DragInt("Detection Priority", &component.DetectionPriority, 1, 0, 100);
    
            ImGui::Separator();
            
            if (ImGui::TreeNode("Perceivable Types"))
            {
                const char* typeNames[] = { "Player", "Enemy", "Neutral", "Environment" };
                for (int i = 0; i < 4; i++)
                {
                    auto it = std::find(component.Types.begin(), component.Types.end(), (PerceivableType)i);
                    bool found = it != component.Types.end();
                    if (ImGui::Checkbox(typeNames[i], &found))
                    {
                        if (found)
                            component.Types.push_back((PerceivableType)i);
                        else
                            component.Types.erase(it);
                    }
                }
                ImGui::TreePop();
            }
            
            if (ImGui::TreeNode("Detection Points"))
            {
                for (int i = 0; i < (int)component.DetectablePointsOffsets.size(); i++)
                {
                    ImGui::PushID(i);
                    ImGui::DragFloat3(("Point " + std::to_string(i)).c_str(), 
                        glm::value_ptr(component.DetectablePointsOffsets[i]), 0.1f);
                    ImGui::SameLine();
                    if (ImGui::SmallButton("X"))
                    {
                        component.DetectablePointsOffsets.erase(component.DetectablePointsOffsets.begin() + i);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                }
                if (ImGui::Button("Add Point"))
                    component.DetectablePointsOffsets.push_back(glm::vec3(0.0f));
        
                ImGui::TreePop();
            }
        });
        DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
        {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            ImGui::DragFloat("Thickness", &component.Thickness, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Fade", &component.Fade, 0.01f, 0.0f, 1.0f);
        });
        DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
        {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
            const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
            
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
                for (int i = 0; i < 3; i++)
                {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                    {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.Type = (Rigidbody2DComponent::BodyType)i;
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
        });
        DrawComponent<Rigidbody3DComponent>("Rigidbody 3D", entity, [](auto& component)
        {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
            const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
            
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
                for (int i = 0; i < 3; i++)
                {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                    {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.Type = (Rigidbody3DComponent::BodyType)i;
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Lock X##Pos", &component.lockPositionX);
                ImGui::Checkbox("Lock Y##Pos", &component.lockPositionY);
                ImGui::Checkbox("Lock Z##Pos", &component.lockPositionZ);
            }

            if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Lock X##Rot", &component.lockRotationX);
                ImGui::Checkbox("Lock Y##Rot", &component.lockRotationY);
                ImGui::Checkbox("Lock Z##Rot", &component.lockRotationZ);
            }
            ImGui::Separator();
            
            ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
            ImGui::DragFloat("Friction", &component.Friction, 0.1f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.1f, 0.0f, 1.0f);
            ImGui::DragFloat("Convex Radius", &component.ConvexRadius, 0.1f, 0.0f, 10.0f);
        });
        DrawComponent<BoxCollider3DComponent>("Box Collider 3D", entity, [](auto& component)
        {
            ImGui::DragFloat3("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat3("Size", glm::value_ptr(component.Size));
            ImGui::Checkbox("Is Trigger", &component.bIsTrigger);
        });
        DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
            ImGui::DragFloat("Density", &component.Density, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.1f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.1f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.1f, 0.0f, 100.0f);
        });
        DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
        {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
            ImGui::DragFloat("Radius", &component.Radius, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Density", &component.Density, 0.1f, 0.0f, 100.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.1f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.1f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.1f, 0.0f, 100.0f);
        });
    }
    template<typename Component>
    void SceneHierarchyPanel::ShowAddComponentEntry(const std::string& name)
    {
        if (!m_SelectedEntity.HasComponent<Component>())
        {
            if (ImGui::MenuItem(name.c_str()))
            {
                m_SelectedEntity.AddComponent<Component>();
                ImGui::CloseCurrentPopup();
            }
        }
    }
}
