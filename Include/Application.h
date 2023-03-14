#ifndef HELLOTRIANGLE_APPLICATION_H
#define HELLOTRIANGLE_APPLICATION_H

#include "Externs.h"

#include <cstdlib>
#include <iostream>
#include <set>


class Application {
private:
    GLFWwindow *window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    /// абстрактный тип поверхности для показа отрендеренных изображений
    VkSurfaceKHR surface;

    VkSwapchainKHR swapChain;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    void createInstance();

    void createLogicalDevice();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();

    void pickPhysicalDevice();

    void createSurface();

    void createSwapChain();

    void initWindow();

    void initVulkan();

    void mainLoop();

    void cleanUp();

    static bool checkValidationLayerSupport();

    static std::vector<const char*> getRequiredExtensions();

    /// Функция для определения того, подойдет ли устройство для наших задач
    bool isDeviceSuitable(VkPhysicalDevice _device);

    static bool checkDeviceExtensionSupport(VkPhysicalDevice _device);

    /// заполняем структуру для проверки 3-х типов свойств \n 1 - Базовые требования (capabilities) surface, такие как
/// мин/макс число изображений в swap chain, мин/макс ширина и высота изображений \n 2 - Формат surface (формат пикселей, цветовое пространство)
/// \n 3 - Доступные режимы работы
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice _device);

    /// Функция для выбора формата surface \n\n
/// Каждый элемент availableFormats содержит члены format и colorSpace. Поле format определяет количество и типы каналов. Например, VK_FORMAT_B8G8R8A8_SRGB обозначает,
/// что у нас есть B, G, R и альфа каналы по 8 бит,
/// всего 32 бита на пиксель. С помощью флага VK_COLOR_SPACE_SRGB_NONLINEAR_KHR в поле colorSpace указывается, поддерживается ли цветовое пространство SRGB.
/// Обратите внимание, что в ранней версии спецификации этот флаг назывался VK_COLORSPACE_SRGB_NONLINEAR_KHR.
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /// Режим работы, пожалуй, самая важная настройка swap chain, поскольку он определяет условия для смены кадров на экране.\n
///Всего в Vulkan доступны четыре режима: \n\n
///VK_PRESENT_MODE_IMMEDIATE_KHR: изображения, отправленные вашим приложением, немедленно отправляются на экран, что может приводить к артефактам.\n\n
///VK_PRESENT_MODE_FIFO_KHR: изображения для вывода на экран берутся из начала очереди в момент обновления экрана. В то время, как программа помещает
/// отрендеренные изображения в конец очереди. Если очередь заполнена, программа будет ждать. Это похоже на вертикальную синхронизацию, используемую в современных играх.\n\n
///VK_PRESENT_MODE_FIFO_RELAXED_KHR: этот режим отличается от предыдущего только в одном случае, когда происходит задержка
/// программы и в момент обновления экрана остается пустая очередь. Тогда изображение передается на экран сразу после его появления
/// без ожидания обновления экрана. Это может привести к видимым артефактам.\n\n
///VK_PRESENT_MODE_MAILBOX_KHR: это еще один вариант второго режима. Вместо того, чтобы блокировать программу при заполнении очереди,
/// изображения в очереди заменяются новыми. Этот режим подходит для реализации тройной буферизации. С ней вы можете
/// избежать появления артефактов при низком времени ожидания. \n\n
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    /// Swap extent – это разрешение изображений в swap chain, которое почти всегда совпадает с разрешением окна (в пикселях), куда рендерятся изображения.
/// Допустимый диапазон мы получили в структуре VkSurfaceCapabilitiesKHR. Vulkan сообщает нам, какое разрешение мы должны выставить,
/// с помощью поля currentExtent (соответствует размеру окна). Однако некоторые оконные менеджеры допускают использование разных разрешений.
/// Для этого указывается специальное значение ширины и высоты в currentExtent — максимальное значение типа uint32_t.
/// В таком случае из промежутка между minImageExtent и maxImageExtent мы выберем разрешение,
/// которое больше всего соответствует разрешению окна. Главное — правильно указать единицы измерения.
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    /// Поиск очереди, поддерживающих графические команды
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice);

public:
    void run();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
};


#endif
