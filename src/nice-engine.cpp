#include <iostream>
#include "fmt/core.h"
#include "vk.h"
#include <array>

using namespace std;
using namespace vk;

void say_hello() {
	auto vulkan = Vulkan::create().unwrap();
	fmt::print("Vulkan {}\n", vulkan->enumerateInstanceVersion());

	fmt::print("extensions:\n");
	auto exts = vulkan->instanceExtProps().unwrap();
	for (auto&& ext : exts) {
		fmt::print("    {} (rev {})\n", ext.name(), ext.specVer());
	}

	fmt::print("layers:\n");
	auto layers = vulkan->instanceLayerProps().unwrap();
	for (auto&& layer : layers) {
		fmt::print("    {} ({} rev {})\n", layer.name(), layer.specVer(), layer.implVer());
	}

	auto instance = Instance::create(vulkan).unwrap();

	// TODO: check for graphics support
	auto pdev = (PhysicalDevice::enumerate(instance).unwrap() | First()).unwrap();
	static_assert(is_same<decltype(pdev), PhysicalDevice>::value);
	// fmt::print("{} device(s) found\n", pdevs | Count());
}
