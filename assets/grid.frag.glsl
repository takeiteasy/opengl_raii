#version 330

// Adapted from:
// https://www.shadertoy.com/view/4tlSRr

out vec4 FragColor;

in vec2 texCoord;
uniform vec2 iResolution;

void main() {
	float rows = 50.0;
	float sizeOfSquares = iResolution.y / rows;

	vec2 vPixelsPerGridSquare = vec2(sizeOfSquares, sizeOfSquares);
	vec2 vScreenPixelCoordinate = fract(texCoord.xy) * vec2(iResolution.y);
	vec2 vGridSquarePixelCoords = fract(vScreenPixelCoordinate / vPixelsPerGridSquare) * vPixelsPerGridSquare;

	vec2 vIsGridLine = step(vGridSquarePixelCoords, vec2(.15));
	vec3 vResult = vec3(max(vIsGridLine.x, vIsGridLine.y));

	if (vResult.r == 0.f)
		FragColor = vec4(0.0f);
	else
		FragColor = vec4(0.f, 1.f, 0.f, 1.0f);
}
