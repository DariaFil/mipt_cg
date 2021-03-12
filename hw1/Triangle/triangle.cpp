// Include standard headers
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <zconf.h>

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 03 - Matrices", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Green background
	glClearColor(0, 0.8, 0, 0);

    // Enable blending
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
    GLuint programIDs[] = {
            LoadShaders( "SimpleTransform.vertexshader", "FirstColor.fragmentshader" ),
            LoadShaders( "SimpleTransform.vertexshader", "SecondColor.fragmentshader" ),
    };

	// Get a handle for our "MVP" uniform
	GLuint MatrixIDs[] = {
            glGetUniformLocation(programIDs[0], "MVP"),
            glGetUniformLocation(programIDs[1], "MVP")
    };

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);

	static const GLfloat g_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,
        -0.5f,  0.8f, 0.0f,
         1.2f,  0.2f, 0.0f,
        -0.2f, -1.5f, 0.0f,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    double rotation = 0.02;
    double camera_angle = 0;

	do{
	    sleep(0.1);
		// Clear the screen
		glClear( GL_COLOR_BUFFER_BIT );

        // Camera matrix
        glm::mat4 View       = glm::lookAt(
                glm::vec3(cos(camera_angle), sin(camera_angle), 4), // Camera is rotating World Space
                glm::vec3(0,0,0), // and looks at the origin
                glm::vec3(0,0,1)  // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around


		for (int i=0; i < 2; ++i) {
            // Use our shader
            glUseProgram(programIDs[i]);

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixIDs[i], 1, GL_FALSE, &MVP[0][0]);

            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(
                    0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void *) 0            // array buffer offset
            );

            // Draw the triangle !
            glDrawArrays(GL_TRIANGLES, i*3, 3); // 3 indices starting at 0 -> 1 triangle

            glDisableVertexAttribArray(0);
        }

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		camera_angle += rotation;

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteProgram(programIDs[0]);
    glDeleteProgram(programIDs[1]);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

