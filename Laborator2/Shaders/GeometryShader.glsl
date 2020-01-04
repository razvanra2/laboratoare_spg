#version 430
layout(lines) in;
//TODO 
//prima data generati o curba (cu line strip), apoi o suprafata de rotatie/translatie cu triangle_strip
layout(line_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;
uniform vec3 control_p1, control_p2, control_p3, control_p4;
uniform int no_of_instances;
uniform int no_of_generated_points;

in int instance[2];


vec3 rotateY(vec3 point, float u)
{
	float x = point.x * cos(u) - point.z *sin(u);
	float z = point.x * sin(u) + point.z *cos(u);
	return vec3(x, point.y, z);
}

vec3 translateX(vec3 point, float t)
{
	return vec3(point.x + t, point.y, point.z);
}

vec3 bezier(float t)
{
	return control_p1 * pow((1 - t), 3) + control_p2 * 3 * t * pow((1 - t), 2) + control_p3 * 3 * pow(t, 2) * (1 - t) + control_p4 * pow(t, 3);
}

//TODO - incercati sa creati si o curba Hermite/Bspline

void main()
{

	if (instance[0] < no_of_instances)
	{
		//TODO 
		//in loc sa emiteti varfuri reprezentand punctele de control, emiteti varfuri care sa aproximeze curba Bezier
		vec3 delta = vec3(1, 0, 0) * instance[0];
		vec3 delta2 = vec3(1, 0, 0) * (instance[0]);
		vec3 delta3 = vec3(1, 0, 0) * (instance[0] + 1);
		vec3 delta4 = vec3(1, 0, 0) * (instance[0] + 1);
		delta2 = rotateY(delta2, 0.05);
		delta4 = rotateY(delta4, 0.05);
		//	glBegin(GL_TRIANGLES);

		for (float t = 0.0; t < 1; t += 1.0 / no_of_generated_points) {
			delta = rotateY(delta, 0.05);
			delta2 = rotateY(delta2, 0.05);
			delta3 = rotateY(delta3, 0.05);
			delta4 = rotateY(delta4, 0.05);
			vec3 p1 = bezier(t);
			vec3 p2 = bezier(t + 1.0 / no_of_generated_points);
			vec3 p3 = bezier(t);
			vec3 p4 = bezier(t + 1.0 / no_of_generated_points);
			
			//glVertex3f(p1);
			//glVertex3f(p2);
			//glVertex3f(p3);
			//glVertex3f(p3);
			//glVertex3f(p4);
			//glVertex3f(p2);


			gl_Position = Projection* View * vec4(p1+delta, 1);	EmitVertex();
			gl_Position = Projection * View * vec4(p2 + delta2, 1);	EmitVertex();
			gl_Position = Projection * View * vec4(p3 + delta3, 1);	EmitVertex();
			gl_Position = Projection * View * vec4(p3 + delta3, 1);	EmitVertex();
			gl_Position = Projection * View * vec4(p4 + delta4, 1);	EmitVertex();
			gl_Position = Projection * View * vec4(p2 + delta2, 1);	EmitVertex();


		}
		//glEnd();

		//gl_Position = Projection* View * vec4(control_p1, 1);	EmitVertex();
		//gl_Position = Projection* View * vec4(control_p2, 1);	EmitVertex();
		//gl_Position = Projection* View * vec4(control_p3, 1);	EmitVertex();
		//gl_Position = Projection* View * vec4(control_p4, 1);	EmitVertex();
		EndPrimitive();
	}
}
