+++
date = '2026-09-22T02:38:59+03:30'
draft = false
title = 'Mercator'
+++

# What maps would look like if the Earth had different poles

On the 4th of September, 2026, the UN General Assembly voted to encourage the world to phase out the Mercator projection and transition to maps using the [Equal Earth projection](https://en.wikipedia.org/wiki/Equal_Earth_projection).

The Earth is, for all intents and purposes, a sphere. Locations are described by their latitude and longitude, measured in degrees or radians. Maps of the world, however, need to be flat, two-dimensional *projections* of the Earth. A projection is simply a mapping from the longitude and latitude \((\lambda, \varphi)\) of each point on Earth to a pair of coordinates \((x, y)\) on a flat, euclidean plane.

We could do this transformation in many ways. We could just set \(x=\lambda\) and \(y=\varphi\), which is what the [equirectangular projection](https://en.wikipedia.org/wiki/Equirectangular_projection) does. We could use a projection by drawing rays from the center of the sphere and seeing where they land on a cylinder wrapped around the globe, then unwrap that cylinder around an arbitrary [prime meridian](https://en.wikipedia.org/wiki/Prime_meridian). This is exactly what cylindrical projections like Mercator do.

One assumption we made was about the \(\lambda\) and \(\varphi\) we feed the projection. The usual longitude and latitude, with 90° North corresponding to the North Pole obviously isn't arbitrary: it's aligned to the rotational axis of the Earth itself. But a projection of a globe doesn't *need* to have the longitude and latitude defined that way.

Another way to think about this is the orientation of the cylinder we're projecting to relative to the globe. If it's parallel to the Earth's rotational axis, it's called a *normal* cylindrical projection. If it's perpendicular to the axis, it's a type of *traverse* cylindrical projection, and putting the cylinder at other angles gives us an *oblique* cylindrical projection.

Below is an interactive demo where you can rotate the Earth and see what a Mercator projection looks like with the cylinder aligned with the screen, and the prime meridian the meridian facing the viewer.

{{< demo "mercator" >}}