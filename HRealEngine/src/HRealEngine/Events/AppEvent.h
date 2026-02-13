
#pragma once

#include "EventBase.h"
#include <vector>
#include <filesystem>
#include <utility>

namespace HRealEngine
{
	class HREALENGINE_API SceneChangeEvent : public EventBase
	{
	public:
		SceneChangeEvent(uint64_t sceneHandle) : m_SceneHandle(sceneHandle) {}
		uint64_t GetSceneHandle() const { return m_SceneHandle; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "SceneChangeEvent: " << m_SceneHandle;
			return ss.str();
		}
		EVENT_CLASS_TYPE(SceneChange)
		EVENT_CLASS_CATEGORY(AppRuntimeEvents)
	private:
		uint64_t m_SceneHandle;
	};
	class HREALENGINE_API WindowResizeEvent : public EventBase
	{
	public:
		WindowResizeEvent(unsigned int widthRef, unsigned int heightRef) : m_Width(widthRef), m_Height(heightRef) {}
		unsigned int GetWidth() const { return m_Width; }
		unsigned int GetHeight() const { return m_Height; }
		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_Width << " x " << m_Height;
			return ss.str();
		}
		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(AppEvents)
	private:
		unsigned int m_Width, m_Height;
	};

	class HREALENGINE_API WindowDropEvent : public EventBase
	{
	public:
		WindowDropEvent(const std::vector<std::filesystem::path>& paths)
			: m_Paths(paths) {}

		WindowDropEvent(std::vector<std::filesystem::path>&& paths)
			: m_Paths(std::move(paths)) {}

		const std::vector<std::filesystem::path>& GetPaths() const { return m_Paths; }

		EVENT_CLASS_TYPE(WindowDrop)
		EVENT_CLASS_CATEGORY(AppEvents)
	private:
		std::vector<std::filesystem::path> m_Paths;
	};
	class HREALENGINE_API WindowCloseEvent : public EventBase
	{
	public:
		WindowCloseEvent() {}
		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
	class HREALENGINE_API AppTickEvent : public EventBase
	{
	public:
		AppTickEvent() {}
		EVENT_CLASS_TYPE(AppTick)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
	class HREALENGINE_API AppUpdateEvent : public EventBase
	{
	public:
		AppUpdateEvent() {}
		EVENT_CLASS_TYPE(AppUpdate)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
	class HREALENGINE_API AppRenderEvent : public EventBase
	{
	public:
		AppRenderEvent() {}
		EVENT_CLASS_TYPE(AppRender)
		EVENT_CLASS_CATEGORY(AppEvents)
	};
}