

# 🌌 OpenGL Renderer

A real-time rendering application using **OpenGL**, featuring **deferred shading**, **PBR-style lighting**, and a dynamic **camera system**. This project implements a programmable rendering pipeline, requiring **manual shader writing** and advanced lighting techniques.
---

## ✨ Features

### 🖥 Rendering Pipeline
- **Deferred Shading** with a **G-buffer** storing:
  - `gPosition`: World-space positions
  - `gNormal`: Normals for lighting calculations
  - `gAlbedoSpec`: Diffuse color and specular intensity
- Separate lighting pass (`shaderLightingPass`)

### 🎥 Camera System
- **Three Camera Modes:**
  - Fixed camera observing the scene
  - Fixed camera tracking a moving sphere
  - First person view moving with the sphere

### 💡 Lighting & Shading
- **Multiple Light Sources:**
  - Point lights, spotlight and directional light (as the sun).
- **Phong & Blinn-Phong Shading Models** (toggleable)
- Specular reflections adjustable via `specularIntensity` & `shininessValue`
- **Fog effects** with smooth transitions
- **Day/Night cycle** and **light attenuation with distance**

### 🌌 Additional Features
- **Skybox** rotating around the scene and changing in a day/night cycle.
- **User Interaction:**
  - Mouse and keyboard input
  - Toggle different shading and camera modes

---

## 📸 Screenshots

<table>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/79b6aed2-5f1f-4b9c-b174-4ebdd8849b75" width="300"></td>
    <td><img src="https://github.com/user-attachments/assets/309d2966-ba6f-4513-b98f-8d77dc3b8c36" width="300"></td>
  </tr>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/fe0b985f-6d61-41ae-8463-bb09beef8b8a" width="300"></td>
    <td><img src="https://github.com/user-attachments/assets/4c67d432-e804-408a-acec-61ad5ddeb564" width="300"></td>
  </tr>
</table>

---

## 🛠 Setup & Compilation

### 📦 Requirements
- **C++** with OpenGL
- **GLEW** & **GLFW**
- **GLM** (for mathematical operations)
- **stb_image.h** (for texture loading)


### 🔧 Building the Project
```bash
git clone https://github.com/uobirek/OpenGL_app.git
cd OpenGL_app
mkdir build && cd build
cmake ..
make
```
### 🎮 Controls

| Key | Action |
|-----|--------|
| `ESC` | Close the window |
| `F` | Follow the sphere (third-person) |
| `G` | Look at the sphere (fixed camera) |
| `H` | Free camera mode (manual control) |
| `W/A/S/D` | Move camera in manual mode |
| `UP/DOWN` | Adjust spotlight direction (manual mode) |
| `R` | Reset spotlight direction |
| `U/I` | Increase/Decrease shininess |
| `J/K` | Increase/Decrease specular intensity |
| `B` | Toggle Blinn-Phong shading |


