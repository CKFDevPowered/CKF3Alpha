//Water Fragment Shader by hzqst

uniform vec4 waterfogcolor;
uniform vec3 eyepos;
uniform float time;
uniform float fresnel;
uniform float abovewater;
uniform sampler2D normalmap;
uniform sampler2D refractmap;
uniform sampler2D reflectmap;
varying vec3 worldpos;
varying vec4 projpos;

void main()
{
	//calculate the normal texcoord and sample the normal vector from texture
	vec2 vNormTexCoord1 = vec2(0.2, 0.15) * time + gl_TexCoord[0].xy; 
	vec2 vNormTexCoord2 = vec2(-0.13, 0.11) * time + gl_TexCoord[0].xy;
	vec2 vNormTexCoord3 = vec2(-0.14, -0.16) * time + gl_TexCoord[0].xy;
	vec2 vNormTexCoord4 = vec2(0.17, 0.15) * time + gl_TexCoord[0].xy;
	vec4 vNorm1 = texture2D(normalmap, vNormTexCoord1);
	vec4 vNorm2 = texture2D(normalmap, vNormTexCoord2);
	vec4 vNorm3 = texture2D(normalmap, vNormTexCoord3);
	vec4 vNorm4 = texture2D(normalmap, vNormTexCoord4);
	vec4 vNormal = vNorm1 + vNorm2 + vNorm3 + vNorm4;
	vNormal = (vNormal / 4.0) * 2.0 - 1.0;

	//calculate texcoord
	vec2 vBaseTexCoord = projpos.xy / projpos.w * 0.5 + 0.5;
	vec2 vOffsetTexCoord = normalize(vNormal.xyz).xy * 0.3;

	//sample the refract color

	vec2 vRefractTexCoord = vBaseTexCoord + vOffsetTexCoord;
	vec4 vRefractColor = texture2D(refractmap, vRefractTexCoord);

	if(abovewater > 0.0)
	{
		//sample the reflect color(texcoord inverted)
		vBaseTexCoord = vec2(projpos.x, -projpos.y) / projpos.w * 0.5 + 0.5;

		vec2 vReflectTexCoord = vBaseTexCoord + vOffsetTexCoord;
		vec4 vReflectColor = texture2D(reflectmap, vReflectTexCoord);

		//fresnel factor, eyepos'z must be larger than worldpos's
		vec3 vEyeVect = eyepos - worldpos;
		float fDist = 1.0 / length(vEyeVect);
		float fFresnel = clamp(vEyeVect.z * fDist * fresnel * 2.0, 0.05, 0.97);

		//lerp the reflection and refraction color by fresnel
		vec4 vFinalColor = vReflectColor * (1.0-fFresnel) + vRefractColor * fFresnel;
		//waterfog color
		vec4 vFinalColor2 = vec4(waterfogcolor.xyz * (vFinalColor.r + vFinalColor.g + vFinalColor.b) / 3.0, 1.0);
		//lerp the final color
		gl_FragColor = vFinalColor2 * waterfogcolor.a + vFinalColor * (1.0-waterfogcolor.a);
	}
	else
	{
		//lerp waterfog color and refraction color
		float fLerp = (vRefractColor.x + vRefractColor.y + vRefractColor.z) / 15.0;
		gl_FragColor = vRefractColor * (1.0 - fLerp) + waterfogcolor * fLerp;
	}
}