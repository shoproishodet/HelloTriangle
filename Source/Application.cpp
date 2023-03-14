#include "Application.h"

/// \createInstanse
/// Это функция которая создает экземпляр VkInstance для инициализации библиотеки - связующее звено между
/// программой и библиотекой Vulkan, здесь для его создания мы предоставляем драйверу сведения о программе
void Application::createInstance() {
    if (g_enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    /// информация о программе предоставляется в следующей структуре (полезная, но не обязательная опция)

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    /// Следующая структура обязательна, она указывает драйверу, какие глобальные расширения и слои валидации мы хотим использовать.
    /// «Глобальные» обозначает, что расширения применяются ко всей программе, а не к конкретному устройству.

    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    /// получаем необходимые глобальные расширения

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    /// экземпляр отладочного мессенджера для vkCreateInstance и vkDestroyInstance
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

    /// передаем информацию о слоях валидации и говорим, какие требуется включить
    if (g_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
        createInfo.ppEnabledLayerNames = g_validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext =  reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (auto err = vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance! err: " + std::to_string(err));
    }

    /// Получаем список поддерживаемых расширений и выводим их в консоль

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensionsP(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsP.data());

    std::cout << "available extensions:\n";

    for (const auto &extension : extensionsP) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

void Application::initWindow() {
    /// инициализируем библиотеку GLFW
    glfwInit();

    /// GLFW была изначально разработана для работы с OpenGL.
    /// Контекст OpenGL нам не нужен, поэтому указываем,
    /// что его создавать не надо, используя следующий вызов:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    /// указываем невозможность изменения размера окна
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    /// создаем окно (4 параметр - монитор, к которому относится окно, 5 параметр -  контекст OpenGL)
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void Application::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

/// явно удаляем все то, что насоздавали

void Application::cleanUp() {
    if (g_enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    vkDestroyDevice(device, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void Application::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanUp();
}

VkBool32 Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

/// заполнение структуры VkDebugUtilsMessengerCreateInfoEXT

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    /// Задаем степени серьезности для вызова callback function
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    /// Фильтрация сообщений по типам (тут все типы, при необходимости отключаем ненужные)
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    createInfo.pfnUserCallback = debugCallback;
}

void Application::setupDebugMessenger() {
    if (!g_enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    /// пытаемся создать экземпляр дебаг мессенджера и проверяем успешна ли была попытка
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

/// Нужно подобрать видеокарту с поддержкой всех необходимых нам функций. Можно выбрать сразу несколько видеокарт и использовать их одновременно.
void Application::pickPhysicalDevice() {
    /// Получаем все девайсы, которые поддерживают вулкан
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    /// выбираем первый девайс, подходящий под наши задачи
    for (const auto &_device : devices) {
        if (isDeviceSuitable(_device)) {
            physicalDevice = _device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

/// создание логического устройства
void Application::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    if (!indices.isComplete())
        throw std::runtime_error("Can't find queue family");

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    ///Каждая очередь имеет приоритет (число с плавающей точкой от 0 до 1),
    /// который влияет на порядок выполнения командных буферов. Приоритет необходимо указать даже в случае, если мы используем всего одну очередь.
    float queuePriority = 1.0f;

    /// задаем необходимое количество очередей для одного семейства (пока что только очередь с поддержкой графических операций)
    for (auto queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    /// Указание используемых возможностей устройства
    VkPhysicalDeviceFeatures deviceFeatures{};
    // сюда я ещё вернусь и заполню это позже

    /// заполнение главной структуры
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    /// включаем расширение VK_KHR_swapchain
    createInfo.enabledExtensionCount = static_cast<uint32_t>(g_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = g_deviceExtensions.data();

    ///В ранних реализациях Vulkan было принято разграничивать слои валидации для экземпляра и для конкретного устройства.
    /// На сегодняшний день такой подход устарел, и поля enabledLayerCount и ppEnabledLayerNames в структуре VkDeviceCreateInfo не учитываются.
    /// Тем не менее, настроим эти параметры для совместимости с более ранними реализациями. Расширения для конкретного устройства нам пока не нужны
    createInfo.enabledExtensionCount = 0;


    if (g_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
        createInfo.ppEnabledLayerNames = g_validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    /// создаем логическое устройство
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    /// получаем дескриптор очереди (девайс, индекс семейства очередей, индекс очереди внутри семейства
    /// (0, тк используем пока только одну очередь, указатель для хранения дескриптора)
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

/// создание поверхности для рендера
void Application::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Application::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;

    /// Битовое поле imageUsage указывает, для каких операций будут использоваться images, полученные из swap chain.
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /// Затем нужно указать, как обрабатывать объекты images, которые используются в нескольких семействах очередей.
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    /// Если у нас несколько очередей, мы будем использовать VK_SHARING_MODE_CONCURRENT. Для этого способа требуется заранее указать,
    /// между какими семействами очередей будет разделено владение.
    /// Это можно сделать с помощью параметров queueFamilyIndexCount и pQueueFamilyIndices.
    /// Если семейство графических очередей и семейство очередей отображения совпадают, что случается чаще, используйте VK_SHARING_MODE_EXCLUSIVE.
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    /// Можно указать, чтобы к изображениям в swap chain применялось какое-либо преобразование из поддерживаемых (supportedTransforms в capabilities),
    /// например, поворот на 90 градусов по часовой стрелке или отражение по горизонтали.
    /// Чтобы не применять никаких преобразований, просто оставьте currentTransform.
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    /// Поле compositeAlpha указывает, нужно ли использовать альфа-канал для смешивания с другими окнами в оконной системе.
    /// Скорее всего, альфа-канал не понадобится, поэтому оставляем VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    /// Поле presentMode говорит само за себя. Если мы выставим VK_TRUE в поле clipped, значит нас не интересуют скрытые пиксели
    /// (например, если часть нашего окна перекрыта другим окном).
    /// Мы всегда сможем выключить clipping, если понадобится прочитать пиксели, а пока оставим clipping включенным.
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    /// Если swap chain станет недействительной, например, из-за изменения размера окна, ее нужно будет воссоздать с нуля
    /// и в поле oldSwapChain указать ссылку на старую swap chain. Пока что трогать это не будем
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
}

bool Application::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : g_validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}

std::vector<const char *> Application::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (g_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return std::move(extensions);
}

bool Application::isDeviceSuitable(VkPhysicalDevice _device) {
    QueueFamilyIndices indices = findQueueFamilies(_device);

    bool extensionsSupported = checkDeviceExtensionSupport(_device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

/// Функция для определения того, подойдет ли устройство для наших задач
bool Application::checkDeviceExtensionSupport(VkPhysicalDevice _device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(g_deviceExtensions.begin(), g_deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);

        if (requiredExtensions.empty())
            return true;
    }

    return false;
}

/// заполняем структуру для проверки 3-х типов свойств \n 1 - Базовые требования (capabilities) surface, такие как
/// мин/макс число изображений в swap chain, мин/макс ширина и высота изображений \n 2 - Формат surface (формат пикселей, цветовое пространство)
/// \n 3 - Доступные режимы работы
SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice _device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

/// Функция для выбора формата surface \n\n
/// Каждый элемент availableFormats содержит члены format и colorSpace. Поле format определяет количество и типы каналов. Например, VK_FORMAT_B8G8R8A8_SRGB обозначает,
/// что у нас есть B, G, R и альфа каналы по 8 бит,
/// всего 32 бита на пиксель. С помощью флага VK_COLOR_SPACE_SRGB_NONLINEAR_KHR в поле colorSpace указывается, поддерживается ли цветовое пространство SRGB.
/// Обратите внимание, что в ранней версии спецификации этот флаг назывался VK_COLORSPACE_SRGB_NONLINEAR_KHR.
VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    if (availableFormats.empty())
        throw std::runtime_error("Vector of available formats is empty!");

    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

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
VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    if (auto it = std::find(availablePresentModes.begin(), availablePresentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != availablePresentModes.end())
        return VK_PRESENT_MODE_MAILBOX_KHR;
    else
        return VK_PRESENT_MODE_FIFO_KHR;
}

/// Swap extent – это разрешение изображений в swap chain, которое почти всегда совпадает с разрешением окна (в пикселях), куда рендерятся изображения.
/// Допустимый диапазон мы получили в структуре VkSurfaceCapabilitiesKHR. Vulkan сообщает нам, какое разрешение мы должны выставить,
/// с помощью поля currentExtent (соответствует размеру окна). Однако некоторые оконные менеджеры допускают использование разных разрешений.
/// Для этого указывается специальное значение ширины и высоты в currentExtent — максимальное значение типа uint32_t.
/// В таком случае из промежутка между minImageExtent и maxImageExtent мы выберем разрешение,
/// которое больше всего соответствует разрешению окна. Главное — правильно указать единицы измерения.
VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

/// Поиск очереди, поддерживающих графические команды
QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice vkPhysicalDevice) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice,
                                             i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        ++i;
    }

    return indices;
}
