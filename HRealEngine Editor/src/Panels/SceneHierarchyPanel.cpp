
#include "SceneHierarchyPanel.h"
#include <imgui/imgui_internal.h>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

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
        auto view = registry.view<TransformComponent, TagComponent>();
        for (auto entity : view)
            DrawEntityNode(Entity{entity, m_Context.get()});

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
            m_SelectedEntity = {};

        if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_SelectedEntity = m_Context->CreateEntity("Empty Entity");
            ImGui::EndPopup();
        }
        
        ImGui::Separator();
        if (m_SelectedEntity)
        {
            ImGui::Begin("Properties");
            if (m_SelectedEntity)
            {
                DrawComponents(m_SelectedEntity);
                
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
        auto& Tag = entity.GetComponent<TagComponent>().Tag;
        
        ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool bOpened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, Tag.c_str());
        if (ImGui::IsItemClicked())
            m_SelectedEntity = entity;

        bool bDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                bDeleted = true;
            ImGui::EndPopup();
        }
        
        if (ImGui::BeginDragDropSource())
        {
            UUID uuid = entity.GetUUID(); 
            ImGui::SetDragDropPayload("SCENE_ENTITY", &uuid, sizeof(UUID));
            ImGui::Text("%s", Tag.c_str());
            ImGui::EndDragDropSource();
        }
        
        if (bOpened)
        {
            ImGuiTreeNodeFlags childFlags = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
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
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

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
            ShowAddComponentEntry<CameraComponent>("Camera Component");
            ShowAddComponentEntry<ScriptComponent>("Script Component");
            ShowAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
            ShowAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
            ShowAddComponentEntry<CircleRendererComponent>("Circle Renderer");
            ShowAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
            ShowAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
            ShowAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();
        
        DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
        {
            DrawVec3Control("Position", component.Position);
            glm::vec3 rotation = glm::degrees(component.Rotation);
            DrawVec3Control("Rotation", rotation);
            component.Rotation = glm::radians(rotation);
            DrawVec3Control("Scale", component.Scale, 1.0f);
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
            ImGui::Button("Texture");
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path fullPath = std::filesystem::path(g_AssetsDirectory) / path;
                    //component.Texture = Texture2D::Create(fullPath.string());
                    Ref<Texture2D> texture = Texture2D::Create(fullPath.string());
                    if (texture->IsLoaded())
                        component.Texture = texture;
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
            ImGui::DragInt("Order In Layer", &component.OrderInLayer, 1, 0, 100);
        });
        DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component)
        {
            // Renk Seçici
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
            
            // Texture Button ve Drag Drop Alanı
            ImGui::Button("Texture");
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path fullPath = std::filesystem::path(g_AssetsDirectory) / path;
                    
                    Ref<Texture2D> texture = Texture2D::Create(fullPath.string());
                    if (texture->IsLoaded())
                        component.Texture = texture;
                }
                ImGui::EndDragDropTarget();
            }

            // Eğer texture yüklüyse texture'ın path'ini veya adını göstermek istersen:
            if (component.Texture)
            {
                ImGui::SameLine();
                ImGui::Text("Loaded"); // İstersen: ImGui::Text(component.Texture->GetPath().c_str());
            }

            // Tiling Factor
            ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
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
