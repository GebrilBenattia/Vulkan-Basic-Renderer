#include "Application.hpp"

int main()
{
    Application App;

    try
    {
        App.Init();
        App.Update();
    }
    catch (const std::exception& E) {
        std::cerr << E.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

	return 0;
}