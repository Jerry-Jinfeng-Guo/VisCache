# VisCache
Unidirectional Path Tracer with Visibility Caching
Hi, this is a (not too) simple unidirectional path tracer with several features. The path tracer was originally developed for the Advanced Graphics course by Jacco Bikker at Utrecht University. Featuring SAH-BVH, NEE, MIS and several BRDF including microfacet model. Pretty much everything was built from scratch. Of course with references to many existing open source solutions, such as PRBT.

The visibility caching is a new feature I finished in October, 2016, for a small research project. The idea is simple, to approximate the visibility term in the rendering equation. Direct illumination will, of course, not benifit from this scheme since it introduces obvious artifact. Indirect illumination, according to many research, varies smoothly and does not have to be physically correct to please human eyes.

With this in mind, we implemented the visibility caching scheme. The implementation is far from optimal. However, we managed to bring down regular shadow ray queries, i.e., by traversing the BVH, by 78%. This is a huge cut.

This repo is merely for exchanging purposes, not meant to be maintained in any form in the future. Please refer to my project report for more details about the project. The report can be easily accessed via my page below.

http://www.students.science.uu.nl/~5610443/

Happy rendering.

- Jerry
