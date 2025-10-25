/**
 * @page pixel_truth The Pixel Truth: Everything is Just Computing RGBA
 *
 * @tableofcontents
 *
 * ## The Fundamental Truth
 *
 * **A pixel has exactly 2 pieces of information:**
 * 1. **Position**: (x, y) - which pixel in the grid
 * 2. **Color**: (R, G, B, A) - red, green, blue, alpha (0-255 each)
 *
 * **That's it. Nothing else can be stored in a pixel.**
 *
 * Everything in graphics - shaders, vertices, VBO, VAO, lighting, shadows, reflections,
 * global illumination - is just **different ways to COMPUTE what RGBA value to put in each pixel**.
 *
 * ---
 *
 * ## The Only Thing That Matters: The Framebuffer
 *
 * ```
 * Framebuffer (1920×1080):
 *   A 2D array of pixels. Each pixel stores:
 *     - R (red):   0-255
 *     - G (green): 0-255
 *     - B (blue):  0-255
 *     - A (alpha): 0-255
 *
 * Example:
 *   framebuffer[500][300] = (255, 0, 0, 255)  ← Red pixel at position (500, 300)
 *   framebuffer[501][300] = (0, 255, 0, 255)  ← Green pixel at position (501, 300)
 *   framebuffer[502][300] = (0, 0, 255, 255)  ← Blue pixel at position (502, 300)
 * ```
 *
 * **This is sent to the monitor. The monitor displays it. Done.**
 *
 * Everything else is just: "How do we compute what RGBA values to put in this array?"
 *
 * ---
 *
 * ## The Question Graphics Solves
 *
 * **Given**:
 * - A 3D scene (cube, lighting, camera position)
 * - A 2D screen (1920×1080 pixels)
 *
 * **Compute**:
 * - For EVERY pixel, what RGBA should it be?
 *
 * ```
 * Input:  3D scene with cube
 * Output: framebuffer[1920][1080] filled with RGBA values
 *
 * framebuffer[0][0] = ?      ← What color should top-left pixel be?
 * framebuffer[1][0] = ?      ← What color should next pixel be?
 * ...
 * framebuffer[1919][1079] = ? ← What color should bottom-right pixel be?
 * ```
 *
 * ---
 *
 * ## Everything is Just RGBA Computation
 *
 * Let's map every graphics concept to: "How does this help compute RGBA?"
 *
 * ### Vertices - Describe Shape Boundaries
 *
 * ```cpp
 * float vertices[] = {
 *     -0.5f, -0.5f, -0.5f,  // Corner 1 of cube
 *      0.5f, -0.5f, -0.5f,  // Corner 2 of cube
 *      0.5f,  0.5f, -0.5f,  // Corner 3 of cube
 * };
 * ```
 *
 * **Purpose**: Define the **boundaries** of shapes in 3D space.
 *
 * **How it helps compute RGBA**:
 * - Vertices define triangles
 * - Triangles define "which pixels are inside the shape"
 * - Only pixels INSIDE get colored
 * - Pixels OUTSIDE stay as background
 *
 * ```
 * Triangle vertices: (100, 100), (200, 100), (150, 200)
 *
 * Rasterizer asks: "Which pixels are inside this triangle?"
 *   Pixel (125, 125)? → YES, inside  → Run shader to compute RGBA
 *   Pixel (50, 50)?   → NO, outside  → Don't compute, leave as background
 *   Pixel (150, 150)? → YES, inside  → Run shader to compute RGBA
 * ```
 *
 * **Vertices don't appear in the final image - they just determine which pixels to color.**
 *
 * ### VBO - Storage for Vertex Data
 *
 * ```cpp
 * glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 * ```
 *
 * **Purpose**: Store vertex data in GPU memory (VRAM).
 *
 * **How it helps compute RGBA**:
 * - GPU needs fast access to vertices
 * - GPU reads VBO to know "what triangles exist"
 * - Triangles determine "which pixels to process"
 * - Only pixels inside triangles get RGBA computed
 *
 * **VBO is just a storage optimization - data could be sent every frame, but that's slow.**
 *
 * ### VAO - Format Description
 *
 * ```cpp
 * glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
 * ```
 *
 * **Purpose**: Tell GPU "how to read the VBO data".
 *
 * **How it helps compute RGBA**:
 * - GPU needs to know: "First 3 floats = position, next 3 floats = color"
 * - Reads position to know where triangle is
 * - Reads color to pass to shader
 * - Shader uses color to compute final RGBA
 *
 * ```
 * VBO: [x, y, z, r, g, b, x, y, z, r, g, b, ...]
 *       ↑
 * VAO: "First 3 = position (x,y,z), next 3 = color (r,g,b), stride = 6 floats"
 *
 * GPU uses this to extract:
 *   Vertex 0: position = (x, y, z), color = (r, g, b)
 *   → Pass to shader to compute RGBA for pixels inside this triangle
 * ```
 *
 * **VAO doesn't affect RGBA - it just helps GPU parse the VBO correctly.**
 *
 * ### Shaders - The RGBA Computation Logic
 *
 * **Vertex Shader** - Transforms vertices to screen space:
 *
 * ```glsl
 * // Input: 3D position from VBO
 * layout (location = 0) in vec3 aPos;
 *
 * void main() {
 *     // Convert 3D world position to 2D screen position
 *     gl_Position = projection * view * model * vec4(aPos, 1.0);
 * }
 * ```
 *
 * **How it helps compute RGBA**:
 * - Takes 3D vertex position (-0.5, -0.5, -0.5)
 * - Applies camera transformation
 * - Outputs 2D screen position (500, 300)
 * - This tells GPU: "This triangle covers pixels near (500, 300)"
 *
 * **Still doesn't compute RGBA! Just positions triangles on screen.**
 *
 * ---
 *
 * **Fragment Shader** - **THIS COMPUTES RGBA!**
 *
 * ```glsl
 * // This runs ONCE per pixel
 * in vec3 fragColor;  // Interpolated from vertices
 * out vec4 finalColor;
 *
 * void main() {
 *     // COMPUTE the RGBA for this pixel
 *     finalColor = vec4(fragColor, 1.0);
 *     // GPU writes: framebuffer[pixel_x][pixel_y] = (R, G, B, A)
 * }
 * ```
 *
 * **THIS IS WHERE RGBA IS COMPUTED!**
 *
 * Every pixel inside a triangle runs this shader.
 * The shader's output (`finalColor`) is written to the framebuffer.
 *
 * ```
 * Pixel at (500, 300):
 *   Fragment shader runs:
 *     fragColor = (1.0, 0.0, 0.0)  ← Red, interpolated from vertices
 *     finalColor = vec4(1.0, 0.0, 0.0, 1.0)
 *   GPU writes:
 *     framebuffer[500][300] = (255, 0, 0, 255)
 * ```
 *
 * **Finally! We computed RGBA for a pixel!**
 *
 * ### Lighting - More Complex RGBA Computation
 *
 * ```glsl
 * // Fragment shader with lighting
 * uniform vec3 lightPos;
 * in vec3 fragNormal;
 * in vec3 fragPos;
 *
 * void main() {
 *     // COMPUTE how much light hits this pixel
 *     vec3 lightDir = normalize(lightPos - fragPos);
 *     float brightness = dot(fragNormal, lightDir);
 *
 *     // COMPUTE final RGBA based on lighting
 *     vec3 baseColor = vec3(1.0, 0.0, 0.0);  // Red
 *     vec3 litColor = baseColor * brightness;
 *     finalColor = vec4(litColor, 1.0);
 *
 *     // GPU writes computed RGBA to framebuffer
 * }
 * ```
 *
 * **How it helps compute RGBA**:
 * - Uses light position and surface normal
 * - Computes "how bright should this pixel be?"
 * - Multiplies base color by brightness
 * - Outputs final RGBA
 *
 * ```
 * Pixel at (500, 300):
 *   Facing light? → brightness = 0.8
 *   Base color: (1.0, 0.0, 0.0) red
 *   Lit color: (0.8, 0.0, 0.0) darker red
 *   framebuffer[500][300] = (204, 0, 0, 255)
 *
 * Pixel at (600, 400):
 *   Facing away from light? → brightness = 0.2
 *   Base color: (1.0, 0.0, 0.0) red
 *   Lit color: (0.2, 0.0, 0.0) very dark red
 *   framebuffer[600][400] = (51, 0, 0, 255)
 * ```
 *
 * **Lighting just changes HOW we compute RGBA. Still ends up as (R, G, B, A) in framebuffer.**
 *
 * ### Textures - Lookup Table for RGBA
 *
 * ```glsl
 * uniform sampler2D textureSampler;
 * in vec2 fragTexCoord;  // (u, v) from 0.0 to 1.0
 *
 * void main() {
 *     // LOOKUP RGBA from texture image
 *     vec4 texColor = texture(textureSampler, fragTexCoord);
 *     finalColor = texColor;
 *     // GPU writes looked-up RGBA to framebuffer
 * }
 * ```
 *
 * **How it helps compute RGBA**:
 * - Texture is a 2D image (e.g., 512×512 pixels with RGBA)
 * - Stored in GPU memory
 * - Fragment shader "looks up" RGBA from texture
 * - Uses texture coordinates (u, v) to find which texel (texture pixel)
 *
 * ```
 * Pixel at (500, 300):
 *   Texture coordinate: (0.25, 0.75)
 *   Lookup texture[128, 384]: (120, 80, 50, 255) ← Wood color
 *   framebuffer[500][300] = (120, 80, 50, 255)
 *
 * Pixel at (501, 300):
 *   Texture coordinate: (0.26, 0.75)
 *   Lookup texture[133, 384]: (125, 85, 55, 255) ← Slightly different wood
 *   framebuffer[501][300] = (125, 85, 55, 255)
 * ```
 *
 * **Texture is just a pre-computed RGBA lookup table.**
 *
 * ### Shadows - Conditional RGBA Darkening
 *
 * ```glsl
 * uniform sampler2D shadowMap;  // Depth texture from light's perspective
 *
 * void main() {
 *     // Is this pixel in shadow?
 *     float shadowDepth = texture(shadowMap, shadowCoord).r;
 *     bool inShadow = (currentDepth > shadowDepth);
 *
 *     // COMPUTE RGBA based on shadow
 *     vec3 baseColor = vec3(1.0, 0.0, 0.0);
 *     vec3 finalColor = inShadow ? baseColor * 0.3 : baseColor;
 *     FragColor = vec4(finalColor, 1.0);
 * }
 * ```
 *
 * **How it helps compute RGBA**:
 * - Shadow map stores depth from light's view
 * - Fragment shader checks: "Can light see this pixel?"
 * - If blocked (in shadow): darken RGBA
 * - If visible: normal RGBA
 *
 * ```
 * Pixel at (500, 300):
 *   In shadow? → YES
 *   Base: (255, 0, 0)
 *   Darkened: (255 * 0.3, 0, 0) = (76, 0, 0)
 *   framebuffer[500][300] = (76, 0, 0, 255)
 * ```
 *
 * **Shadows just multiply RGBA by a darker value. Still ends up as RGBA.**
 *
 * ### Global Illumination - Even More Complex RGBA Computation
 *
 * ```glsl
 * // Extremely simplified
 * void main() {
 *     // Direct lighting
 *     vec3 direct = computeDirectLight();
 *
 *     // Indirect lighting (light bouncing from other surfaces)
 *     vec3 indirect = computeIndirectLight();  // Very expensive!
 *
 *     // Ambient occlusion (corners are darker)
 *     float ao = computeAmbientOcclusion();
 *
 *     // COMPUTE final RGBA
 *     vec3 finalColor = (direct + indirect) * ao;
 *     FragColor = vec4(finalColor, 1.0);
 * }
 * ```
 *
 * **How it helps compute RGBA**:
 * - Traces light bouncing around the scene
 * - Computes accumulated light at this pixel
 * - More realistic, but much more computation
 *
 * ```
 * Pixel at (500, 300):
 *   Direct light: (0.5, 0.0, 0.0)
 *   Bounce from blue wall: (0.0, 0.0, 0.1)
 *   Ambient occlusion: 0.8 (slightly occluded)
 *   Final: (0.5 + 0.0, 0.0 + 0.0, 0.0 + 0.1) * 0.8 = (0.4, 0.0, 0.08)
 *   framebuffer[500][300] = (102, 0, 20, 255)
 * ```
 *
 * **Global illumination is VERY expensive RGBA computation. But still: just RGBA.**
 *
 * ---
 *
 * ## The Complete Mapping
 *
 * ```
 * ┌─────────────────────────────────────────────────────────────┐
 * │ GOAL: Fill framebuffer with RGBA values                    │
 * └─────────────────────────────────────────────────────────────┘
 *                          ↑
 *          ┌───────────────┴────────────────┐
 *          │                                │
 *     What to compute?              How to compute it?
 *          │                                │
 *          ↓                                ↓
 * ┌──────────────────┐           ┌──────────────────────┐
 * │ Geometry         │           │ Shaders              │
 * │ (Vertices)       │           │ (RGBA computation)   │
 * │                  │           │                      │
 * │ Purpose:         │           │ Purpose:             │
 * │ Define which     │           │ Compute RGBA for     │
 * │ pixels to color  │           │ each pixel           │
 * │                  │           │                      │
 * │ Triangle at:     │           │ Simple:              │
 * │ (100,100),       │           │   return red;        │
 * │ (200,100),       │           │                      │
 * │ (150,200)        │           │ Lighting:            │
 * │   ↓              │           │   return red * light;│
 * │ Covers 5000      │           │                      │
 * │ pixels           │           │ Texture:             │
 * │   ↓              │           │   return texture();  │
 * │ Run shader       │           │                      │
 * │ 5000 times       │           │ Shadows:             │
 * └────────┬─────────┘           │   return darkened;   │
 *          │                     │                      │
 *          │                     │ Global Illumination: │
 *          │                     │   return bounced;    │
 *          │                     └──────────┬───────────┘
 *          │                                │
 *          └────────────┬───────────────────┘
 *                       ↓
 *            ┌──────────────────────┐
 *            │ Fragment Shader      │
 *            │ Runs per pixel       │
 *            │ Outputs RGBA         │
 *            └──────────┬───────────┘
 *                       ↓
 *            ┌──────────────────────┐
 *            │ Framebuffer          │
 *            │ [x][y] = (R,G,B,A)   │
 *            └──────────┬───────────┘
 *                       ↓
 *            ┌──────────────────────┐
 *            │ Monitor              │
 *            │ Displays pixels      │
 *            └──────────────────────┘
 * ```
 *
 * ---
 *
 * ## Mental Model: The Two-Step Process
 *
 * ### Step 1: Which Pixels? (Geometry)
 *
 * **Input**: Vertices (triangles)
 * **Process**: Rasterization
 * **Output**: List of pixel coordinates to process
 *
 * ```
 * Vertices define triangle: (100,100), (200,100), (150,200)
 *   ↓ Rasterizer
 * Pixel list: (100,100), (101,100), (102,100), ..., (150,175), ... (5000 pixels)
 * ```
 *
 * **VBO, VAO, vertex shader - all part of this step.**
 *
 * ### Step 2: What RGBA? (Shading)
 *
 * **Input**: List of pixels from Step 1
 * **Process**: Fragment shader (YOUR code!)
 * **Output**: RGBA for each pixel
 *
 * ```
 * For pixel (125, 135):
 *   Fragment shader runs:
 *     vec4 color = computeColor();  ← YOUR logic here
 *     return color;
 *   ↓
 * framebuffer[125][135] = (R, G, B, A)
 * ```
 *
 * **Everything else (lighting, textures, shadows, GI) - just ways to compute RGBA.**
 *
 * ---
 *
 * ## Why This Matters
 *
 * Understanding "it's all just RGBA" clarifies:
 *
 * ### 1. Why shaders are so powerful
 *
 * You control the RGBA computation:
 * ```glsl
 * // Simple: solid color
 * FragColor = vec4(1.0, 0.0, 0.0, 1.0);
 *
 * // Complex: realistic lighting
 * FragColor = computePhysicallyBasedLighting();
 *
 * // Artistic: toon shading
 * FragColor = quantize(color);  // Cel shading
 * ```
 *
 * ### 2. Why GPUs are so fast
 *
 * 2 million pixels need RGBA computed.
 * GPU runs fragment shader 2 million times in parallel.
 * All writing to different framebuffer locations - no conflicts!
 *
 * ### 3. Why framebuffers exist
 *
 * You can't write to the monitor directly.
 * Framebuffer is the intermediate RGBA storage.
 * GPU fills it, display engine reads it.
 *
 * ### 4. Why vertices matter
 *
 * Without vertices, GPU doesn't know WHICH pixels to compute RGBA for.
 * Vertices define the geometry boundaries.
 *
 * ### 5. Why VBO/VAO are just optimization
 *
 * They don't affect RGBA directly.
 * They just make accessing vertex data faster.
 * Faster access → faster "which pixels?" step → faster rendering.
 *
 * ---
 *
 * ## The Simplest Possible Renderer
 *
 * If you could write to framebuffer directly (you can't in OpenGL, but conceptually):
 *
 * ```cpp
 * // Direct pixel writing (pseudo-code)
 * for (int y = 0; y < 1080; y++) {
 *     for (int x = 0; x < 1920; x++) {
 *         // Compute RGBA for this pixel
 *         RGBA color = myComputeFunction(x, y);
 *         framebuffer[x][y] = color;
 *     }
 * }
 * sendToMonitor(framebuffer);
 * ```
 *
 * **That's it! That's all rendering is!**
 *
 * Everything in OpenGL is just making this faster:
 * - Vertices: Skip pixels outside shapes
 * - VBO: Fast vertex access
 * - VAO: Efficient vertex format parsing
 * - Shaders: Parallel RGBA computation on GPU
 * - Textures: Pre-computed color lookup
 *
 * ---
 *
 * ## The Ultimate Truth: It's All Just a Function
 *
 * Every rendering technique, no matter how complex, is just:
 *
 * ```cpp
 * // The universal signature of ALL graphics:
 * RGBA compute_pixel_color(int x, int y);
 *
 * // Different techniques = different implementations
 *
 * // Technique 1: Flat color (trivial)
 * RGBA compute_pixel_color(int x, int y) {
 *     return {255, 0, 0, 255};  // Always red
 * }
 *
 * // Technique 2: Texture lookup (simple)
 * RGBA compute_pixel_color(int x, int y) {
 *     vec2 uv = calculate_uv(x, y);
 *     return texture_lookup(button_image, uv);
 * }
 *
 * // Technique 3: Phong lighting (medium)
 * RGBA compute_pixel_color(int x, int y) {
 *     vec3 normal = get_surface_normal(x, y);
 *     vec3 light_dir = normalize(light_pos - pixel_pos);
 *     float brightness = dot(normal, light_dir);
 *     return base_color * brightness;
 * }
 *
 * // Technique 4: Ray tracing (complex)
 * RGBA compute_pixel_color(int x, int y) {
 *     Ray ray = create_ray_from_camera_through_pixel(x, y);
 *     Object hit = trace_ray(ray);
 *
 *     if (hit.exists) {
 *         vec3 color = hit.material.color;
 *
 *         // Shadow ray
 *         Ray shadow_ray = create_ray_to_light(hit.point);
 *         if (is_blocked(shadow_ray)) {
 *             color *= 0.3;  // Darken
 *         }
 *
 *         // Reflection ray
 *         Ray reflection = reflect(ray, hit.normal);
 *         vec3 reflected_color = trace_ray(reflection);
 *         color = blend(color, reflected_color, hit.material.reflectivity);
 *
 *         return to_rgba(color);
 *     }
 *     return {0, 0, 0, 255};  // Black background
 * }
 *
 * // Technique 5: Path tracing (very complex)
 * RGBA compute_pixel_color(int x, int y) {
 *     vec3 accumulated_color = {0, 0, 0};
 *
 *     // Sample many rays for noise reduction
 *     for (int sample = 0; sample < 1000; sample++) {
 *         Ray ray = create_ray_from_camera(x, y, random_offset());
 *
 *         // Bounce ray multiple times
 *         for (int bounce = 0; bounce < 10; bounce++) {
 *             Object hit = trace_ray(ray);
 *             if (!hit.exists) break;
 *
 *             // Accumulate light contribution
 *             accumulated_color += hit.emission;
 *
 *             // Bounce in random direction (Monte Carlo)
 *             ray = create_random_bounce(hit.point, hit.normal);
 *         }
 *     }
 *
 *     // Average all samples
 *     accumulated_color /= 1000;
 *     return to_rgba(accumulated_color);
 * }
 * ```
 *
 * **Same signature. Same output. Different computation complexity.**
 *
 * ```
 * ALL TECHNIQUES CONVERGE TO THE SAME THING:
 *
 *   Flat Color ───┐
 *   Texture ──────┤
 *   Phong ────────┼──→ framebuffer[y][x] = {R, G, B, A} ──→ Monitor
 *   Ray Tracing ──┤
 *   Path Tracing ─┘
 *
 * COMPLEXITY COMPARISON:
 *   Flat:         1 operation per pixel
 *   Texture:      ~10 operations per pixel
 *   Phong:        ~100 operations per pixel
 *   Ray Tracing:  ~10,000 operations per pixel
 *   Path Tracing: ~100,000,000 operations per pixel
 *
 * OUTPUT: All produce 4 bytes (RGBA)
 * ```
 *
 * **Why does complexity matter?**
 *
 * Higher complexity = more realistic lighting:
 * - Flat: Looks like a cartoon
 * - Phong: Looks like a video game from 2005
 * - Ray tracing: Looks like a modern AAA game
 * - Path tracing: Looks like a photograph (if you wait long enough!)
 *
 * But the framebuffer doesn't care. It just stores RGBA.
 *
 * ---
 *
 * ## The Rendering Loop (All Techniques)
 *
 * ```cpp
 * // Pseudocode for ALL rendering, regardless of technique:
 *
 * void render_frame() {
 *     // Step 1: Clear framebuffer
 *     for (int y = 0; y < HEIGHT; y++) {
 *         for (int x = 0; x < WIDTH; x++) {
 *             framebuffer[y][x] = {0, 0, 0, 255};  // Black
 *         }
 *     }
 *
 *     // Step 2: For each object, determine which pixels it covers
 *     for (Object obj : scene) {
 *         PixelList pixels = rasterize(obj);  // Which pixels?
 *
 *         // Step 3: For each pixel, compute color
 *         for (Pixel p : pixels) {
 *             RGBA color = compute_pixel_color(p.x, p.y);
 *             framebuffer[p.y][p.x] = color;
 *         }
 *     }
 *
 *     // Step 4: Send to monitor
 *     send_to_display(framebuffer);
 * }
 *
 * // The ONLY thing that changes between techniques is:
 * //   compute_pixel_color(x, y)
 * //
 * // Everything else is the same!
 * ```
 *
 * ---
 *
 * ## What About VBO, VAO, Shaders?
 *
 * They're all just **implementation details** of the two core operations:
 *
 * ### Operation 1: "Which pixels to process?"
 *
 * ```
 * Vertices ──→ VBO (storage) ──→ VAO (format) ──→ Vertex Shader (position)
 *                                                        ↓
 *                                                   Rasterizer
 *                                                        ↓
 *                                            List of (x,y) coordinates
 * ```
 *
 * **Result:** List of pixel coordinates to color
 *
 * ### Operation 2: "What color for each pixel?"
 *
 * ```
 * Fragment Shader (YOUR CODE):
 *   input:  (x, y)
 *   output: (R, G, B, A)
 *
 * Executed in parallel by GPU for all pixels.
 * ```
 *
 * **Result:** RGBA value for each pixel
 *
 * ### They Combine:
 *
 * ```
 * for (Pixel p : rasterized_pixels) {
 *     framebuffer[p.y][p.x] = fragment_shader(p.x, p.y);
 * }
 * ```
 *
 * **That's it. That's ALL rendering.**
 *
 * ---
 *
 * ## Why GPUs Are Fast
 *
 * Because this loop:
 * ```cpp
 * for (int i = 0; i < 2_000_000; i++) {
 *     framebuffer[i] = compute_pixel_color(i);
 * }
 * ```
 *
 * Can be parallelized:
 * ```
 * CPU (8 cores):
 *   Core 0: framebuffer[0..250000] = compute...
 *   Core 1: framebuffer[250001..500000] = compute...
 *   ...
 *   Core 7: framebuffer[1750000..2000000] = compute...
 *
 *   Still slow - only 8 pixels at once.
 *
 * GPU (96 execution units × 7 threads = 672 parallel):
 *   EU 0:  framebuffer[0..2975] = compute...
 *   EU 1:  framebuffer[2976..5951] = compute...
 *   ...
 *   EU 95: framebuffer[285714..288689] = compute...
 *
 *   672 pixels computed simultaneously!
 * ```
 *
 * **GPU does the SAME work, just in parallel.**
 *
 * ---
 *
 * ## The Complete Mental Model
 *
 * ```
 * ┌─────────────────────────────────────────────────────┐
 * │ YOUR GOAL: Fill this 2D array                      │
 * │                                                     │
 * │ framebuffer[1080][1920] = {                        │
 * │   {R, G, B, A}, {R, G, B, A}, ... (1920 pixels)    │
 * │   {R, G, B, A}, {R, G, B, A}, ... (1920 pixels)    │
 * │   ... (1080 rows)                                  │
 * │ }                                                   │
 * └──────────────────┬──────────────────────────────────┘
 *                    │
 *         ┌──────────┴──────────┐
 *         │                     │
 *    Question 1           Question 2
 *    WHICH pixels?        WHAT color?
 *         │                     │
 *         ↓                     ↓
 *   ┌──────────┐         ┌──────────────┐
 *   │ Geometry │         │ Shader       │
 *   │ (Vertices│         │ (Your        │
 *   │  VBO/VAO)│         │  RGBA logic) │
 *   └────┬─────┘         └──────┬───────┘
 *        │                      │
 *        ↓                      │
 *   Rasterizer                  │
 *   (GPU HW)                    │
 *        │                      │
 *        ↓                      │
 *   Pixel list:                 │
 *   (100,100)  ────────────────┐│
 *   (101,100)  ────────────────┤│
 *   (102,100)  ────────────────┤│
 *   ...        ────────────────┤│
 *                              ││
 *   ┌──────────────────────────┘│
 *   │ For each pixel:           │
 *   │   RGBA = shader(x, y) ←───┘
 *   │   framebuffer[y][x] = RGBA
 *   └──────────┬────────────
 *              ↓
 *      ┌────────────────┐
 *      │ Display Engine │
 *      │ Reads 60Hz     │
 *      └───────┬────────┘
 *              │ HDMI
 *              ↓
 *          [Monitor]
 * ```
 *
 * **Everything else is optimization or artistic choice.**
 *
 * ---
 *
 * ## Summary
 *
 * | Concept | Purpose | Affects RGBA? |
 * |---------|---------|---------------|
 * | **Vertices** | Define shape boundaries | No - determines WHICH pixels |
 * | **VBO** | Store vertices in VRAM | No - just storage optimization |
 * | **VAO** | Describe vertex format | No - just parsing metadata |
 * | **Vertex Shader** | Transform vertices to screen | No - just positioning |
 * | **Rasterizer** | Find pixels in triangles | No - just pixel list |
 * | **Fragment Shader** | **COMPUTE RGBA** | **YES! This is where RGBA comes from!** |
 * | **Textures** | Lookup pre-computed RGBA | Yes - provides input to shader |
 * | **Lighting** | Math for RGBA computation | Yes - multiplies/adds to RGBA |
 * | **Shadows** | Conditional RGBA darkening | Yes - modifies RGBA |
 * | **Global Illumination** | Complex RGBA math | Yes - advanced RGBA computation |
 * | **Framebuffer** | Store final RGBA | No - just storage |
 * | **Monitor** | Display RGBA | No - just display device |
 *
 * **Everything exists to answer two questions:**
 * 1. **Which pixels should have color?** (Vertices, rasterization)
 * 2. **What RGBA should each pixel be?** (Fragment shader, everything else)
 *
 * The framebuffer stores the answers. The monitor displays them.
 *
 * **That's all graphics is.**
 *
 * ---
 *
 * ## See Also
 *
 * - @ref display_fundamentals - How pixels get on screen
 * - @ref hardware_pipeline - Hardware execution flow
 * - @ref opengl_tutorial - OpenGL API details
 */
