# Systems

### Input System
**Read/Write**
	* Read & Write - InputComponent - Shared

**Description**
	* Polls GLFW for events which triggers the appropriate callbacks (key, cursor or mouse)
	* Updates the shared input component which can be accessed by any other systems

### Player Move System
**Read/Write**
	* Read 	- InputComponent 		- Shared
	* Write 	- TransformComponent - Non-shared
	* Write 	- MotionComponent 	- Non-shared

**Description**
	* Updates the velocity of an entity that has an input, transform and motion component (a player)
	* Updates the rotation of the player transform based on mouse input

### Movement System
**Read/Write**
	* Write 	- TransformComponent - Non-shared
	* Read 	- MotionComponent 	- Non-shared

**Description**
	* Updates the transform of any entity with a transform and a motion component based on its velocity 
	and the delta time

### CameraSystem
**Read/Write**
	* Read 	- TransformComponent	- Non-shared
	* Write 	- CameraComponent		- Shared

**Description**
* Checks if only one entity has a camera component
* Updates the shared camera component (view & projection matrix) based on the current transform

### Render System
**Read/Write**
	* Read - TransformComponent 	- Non-shared
	* Read - RenderableComponent 	- Non-shared

**Description**
* Uses the transform component of the entity to create a model matrix as a constant buffer
* Uses the buffers stored in the renderable component to render the object