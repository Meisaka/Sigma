
#version 330 core

in vec2 ex_UV;
out vec4 out_Color;

uniform sampler2D in_Texture;
uniform vec4 kfact;
uniform vec4 cfact;
uniform vec2 ScaleIn;
uniform vec2 ScaleOut;
uniform vec4 LensCenter;
uniform vec4 ScreenCenter;

vec4 HmdWarp(vec2 inuv, vec2 Lens, vec2 Screen) {
	vec2 theta = (inuv - Lens) * ScaleIn;
	float rSq = (theta.x * theta.x) + (theta.y * theta.y);
	vec2 rvector = theta * (kfact.x + (kfact.y * rSq) + (kfact.z * rSq * rSq) + (kfact.w * rSq * rSq * rSq));
	vec2 tlookup = (Lens + (ScaleOut * rvector));
	vec2 clim = clamp(tlookup, Screen - vec2(0.25,0.5), Screen + vec2(0.25, 0.5)) - tlookup;
	if(clim.x < 0.0 || clim.x > 0.0 || clim.y < 0.0 || clim.y > 0.0) {
	discard;
	}
	return texture(in_Texture, tlookup);
}

vec4 HmdCWarp(vec2 inuv, vec2 Lens, vec2 Screen) {
	vec2 theta = (inuv - Lens) * ScaleIn;
	float rSq = (theta.x * theta.x) + (theta.y * theta.y);
	vec2 rvector = theta * (kfact.x + (kfact.y * rSq) + (kfact.z * rSq * rSq) + (kfact.w * rSq * rSq * rSq));
	vec2 thetaB = rvector * (cfact.z + cfact.w * rSq);
	vec2 tcBlue = Lens + (ScaleOut * thetaB);
	vec2 clim = clamp(tcBlue, Screen - vec2(0.25,0.5), Screen + vec2(0.25, 0.5)) - tcBlue;
	if(clim.x < 0.0 || clim.x > 0.0 || clim.y < 0.0 || clim.y > 0.0) {
	discard;
	}
	vec2 tcGreen = Lens + (ScaleOut * rvector);
	vec2 thetaR = rvector * (cfact.x + cfact.y * rSq);
	vec2 tcRed = Lens + (ScaleOut * thetaR);
	float blue = texture(in_Texture, tcBlue).b;
	float green = texture(in_Texture, tcGreen).g;
	float red = texture(in_Texture, tcRed).r;
	return vec4(red, green, blue, 1.0);
}

void main(void) {
	vec2 tf_UV;
	vec2 warp_UV;
	vec4 tccolor;
	if(ex_UV.x < 0.5) {
	tf_UV = vec2(ex_UV.x * 1.0, ex_UV.y);
	tccolor = HmdCWarp(tf_UV, LensCenter.xy, ScreenCenter.xy);
	//tccolor = texture(in_Texture, tf_UV);
	}
	else {
	tf_UV = vec2((ex_UV.x), ex_UV.y);
	tccolor = HmdCWarp(tf_UV, LensCenter.zw, ScreenCenter.zw);
	//tccolor = texture(in_Texture, tf_UV);
	}
	out_Color = vec4(tccolor.rgb,1.0);
}

