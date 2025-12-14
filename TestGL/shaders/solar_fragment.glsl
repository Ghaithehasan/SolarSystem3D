#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 viewPos;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D nightTexture;
uniform sampler2D cloudsTexture;
uniform bool useTexture;
uniform bool useNightTexture;
uniform bool useCloudsTexture;

// Sun light
uniform vec3 sunPos;
uniform vec3 sunColor;
uniform float sunIntensity;

// Moon light
uniform vec3 moonPos;
uniform vec3 moonColor;
uniform float moonIntensity;
uniform bool isMoon;

// Object type: 0 = Sun, 1 = Earth, 2 = Moon
uniform int objectType;

void main() {
    vec3 color = objectColor;
    
    // Sun emits its own light
    if (objectType == 0) {
        vec3 sunGlow = vec3(1.0, 0.95, 0.8) * 2.0;
        if (useTexture) {
            vec3 sunTex = texture(diffuseTexture, TexCoord).rgb;
            sunGlow = sunTex * 2.5;
        }
        FragColor = vec4(sunGlow, 1.0);
        return;
    }
    
    vec3 result = vec3(0.0);
    vec3 norm = normalize(Normal);
    
    // Get base color from texture or object color
    vec3 baseColor = objectColor;
    if (useTexture) {
        baseColor = texture(diffuseTexture, TexCoord).rgb;
    }
    
    // Sun lighting
    vec3 sunDir = normalize(sunPos - FragPos);
    float sunDist = length(sunPos - FragPos);
    float sunDiff = max(dot(norm, sunDir), 0.0);
    
    // Ambient from sun
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * sunColor * sunIntensity;
    
    // Diffuse from sun
    vec3 sunDiffuse = sunDiff * sunColor * sunIntensity / (1.0 + sunDist * 0.01);
    
    // Specular from sun
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-sunDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 sunSpecular = 0.3 * spec * sunColor * sunIntensity;
    
    result += (ambient + sunDiffuse + sunSpecular) * baseColor;
    
    // Add clouds texture for Earth
    if (useCloudsTexture && objectType == 1) {
        vec3 clouds = texture(cloudsTexture, TexCoord).rgb;
        float cloudAlpha = clouds.r * 0.3;
        result = mix(result, clouds, cloudAlpha);
    }
    
    // Add night texture for Earth (dark side)
    if (useNightTexture && objectType == 1) {
        float sunLight = max(dot(norm, sunDir), 0.0);
        if (sunLight < 0.3) {
            vec3 nightColor = texture(nightTexture, TexCoord).rgb;
            result = mix(result, nightColor, (0.3 - sunLight) / 0.3);
        }
    }
    
    // Moon lighting (faint, only when sun is not visible)
    if (!isMoon && objectType == 1) { // Only Earth receives moon light
        vec3 moonDir = normalize(moonPos - FragPos);
        float moonDist = length(moonPos - FragPos);
        float moonDiff = max(dot(norm, moonDir), 0.0);
        
        // Check if moon is visible (not in sun's shadow)
        float sunMoonAngle = dot(sunDir, moonDir);
        if (sunMoonAngle < 0.2) { // Moon is on the opposite side from sun
            vec3 moonAmbient = 0.08 * moonColor * moonIntensity;
            vec3 moonDiffuse = moonDiff * moonColor * moonIntensity / (1.0 + moonDist * 0.05);
            result += (moonAmbient + moonDiffuse) * baseColor * 0.4;
        }
    }
    
    // Moon emits faint light (visible when sun is not shining on it)
    if (objectType == 2) {
        vec3 sunToMoon = normalize(moonPos - sunPos);
        vec3 moonNormal = normalize(FragPos - moonPos);
        float sunOnMoon = max(dot(moonNormal, sunToMoon), 0.0);
        
        // Moon glows faintly, more visible when not directly lit by sun
        vec3 moonGlow = vec3(0.9, 0.9, 0.95) * (0.2 + 0.1 * (1.0 - sunOnMoon));
        if (useTexture) {
            vec3 moonTex = texture(diffuseTexture, TexCoord).rgb;
            result = moonTex * (ambient + sunDiffuse) + moonGlow * 0.3;
        } else {
            result = baseColor * (ambient + sunDiffuse) + moonGlow;
        }
    }
    
    FragColor = vec4(result, 1.0);
}

