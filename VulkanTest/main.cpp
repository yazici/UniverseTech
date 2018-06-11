#include <iostream>
#include <stdexcept>


#include "VulkanWinApp.h"

int main() {
	VulkanWinApp app;

	app.run();

	/*try {
		app.run();
	} catch(const std::runtime_error& e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
*/
	return EXIT_SUCCESS;
}