# spherical-harmonics-visualization-vulkan
A Vulkan application to play around with and visualize spherical harmonics. The application makes use of the [spherical-harmonics library](https://github.com/google/spherical-harmonics) to:
* Visualize an arbitrary spherical function
* Visualize the projection of this spherical function into SH basis functions weighted by their coefficients
* Visualize the reconstruction of the arbitrary spherical function by taking the weighted sum of these basis functions 


# Build (currently Windows Visual Studio only)
Clone the project, download the [dependencies](https://drive.google.com/drive/folders/11RiEnKvYco3RQDe-qgo2Ftz8tPNr44Kh?usp=sharing) and place the `Libraries` folder in the `./spherical-harmonics-visualization` directory. You should now be able to open up the solution (`spherical-harmonics-visualization/spherical-harmonics-visualization.sln`) in Visual Studio and build and run the project.

# Usage
## Changing the spherical function that is visualized
In `vvt_app.cpp`, you will find the implementation of `initVisualizations()`:
```cpp
	void VvtApp::initVisualizations()
	{
		std::shared_ptr<VvtModel> pointModel = VvtModel::createModelFromFile(vvtDevice, "../Models/sphere.obj");

		sh::SphericalFunction func = [](double phi, double theta) { return glm::sin(phi) * glm::cos(phi); };
		SphereContainer sphereFunc1 = { {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f }, 3.0f, func, pointModel };
		sphereFunctions.push_back(sphereFunc1);
		sphereFunctions.back().generateSpherePoints();
	}
```
You can change the implementation of the lambda function `sh::SphericalFunction func` to the spherical function that you want to visualize. I plan on making a function parser in the future so this can be done dynamically in the UI window instead of having to manually change this in the code each time you want to visualize a different function.
## User input
The application features a small UI window which allows you to rotate the spherical function and its reconstruction using XYZ Euler angles. Furthermore the user is able to move through the scene using WASD and tilt the camera using the arrow keys.

# Example visualization
![Thumbnail](./thumbnail.png?raw=true "Example visualization")

# Future work
* Build support for UNIX systems
* Dynamic updating of the spherical function, without having to manually change it in the code
* UI upgrades
