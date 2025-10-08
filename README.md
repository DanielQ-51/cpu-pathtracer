# CPU PBR Path Tracer
## Written in C++ from scratch by Daniel Qin (qindan@seas.upenn.edu)
*Development: mid August 2025 – early October 2025*

<img src="demoimages/redbluelighting.png" alt="cool image of some red and blue lights" width="50%"/>
<br>
This path tracer currently supports three main classes of materials: a diffuse lambertian material (cosine weighted), a mirror material, and a metal-like material (Phong). These can be seen in the images below:
<br>
<div style="display: flex; gap: 5%; align-items: center;">
  <img src="demoimages/metalboxes.png" alt="metal boxes" width="30%">
  <img src="demoimages/mirrorBox.png" alt="mirror and diffuse boxes" width="30%">
</div>
<br>
One of the cooler things about this path tracer is that it uses Multiple Importance Sampling (MIS) to accurately combine BSDF sampling with direct light sampling (Next Event Estimation). Direct sampling of the lights allows the path tracer to accurately handle small light sources. With only BSDF sampling, which is where you hope that your bouncing rays happen to find a light source, small light sources are very hard to get right. Below is a comparison of only BSDF sampling versus MIS combining BSDF and light sampling. I hope you can tell which is which!
<br>
<div style="display: flex; gap: 5%; align-items: center;">
  <img src="demoimages/badexample.png" alt="ugly render" width="30%">
  <img src="demoimages/smallLightMirrorBoxes.png" alt="beautiful render" width="30%">
</div>
<br>
With our MIS, we are able to handle more complex lighting with multiple light sources of different sizes. For example, the following scene has a ton of different lights, including some behind the camera.
<br>
<img src="demoimages/saintquartz.png" alt="very cool render with many lights" width="50%"/>
<br>
One other thing is that, as you may have noticed in the above image, there are a lot of interesting shapes in the scene. That's because this path tracer renders based on triangles, and is able to read in .obj files, convert all quads and n-gons to triangles, and render the resulting mesh. Another cool thing is that the colors of triangles are indeed interpolated between their vertices. The stellated tetrahedron in the center of the scene simply has a color assigned to each of its 8 points. Here's a clearer picture:

<img src="demoimages/saintquartz3.png" alt="its a saint quartz from the hit game FGO" width="50%"/>

For any questions, please email or message me on linkedIn. I would be more than glad to answer any and all questions.


