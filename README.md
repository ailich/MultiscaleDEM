README
================
Alexander Ilich
October 18, 2021

# MultiscaleDEM

## THIS IS A PRE-RELEASE. IT IS STILL BEING TESTED AND FUNCTIONS MAY RECEIVE MAJOR CHANGES.

[![DOI](https://zenodo.org/badge/353158828.svg)](https://zenodo.org/badge/latestdoi/353158828)

Please cite as

Ilich, Alexander R.; Lecours, Vincent; Misiuk, Benjamin; Murawski,
Steven A.; 2021. “MultiscaleDEM”, <doi:10.5281/zenodo.5548338>.
<https://github.com/ailich/MultiscaleDEM>.

## Purpose

This package calculates multi-scale geomorphometric terrain attributes
from regularly gridded DEM/bathymetry rasters.

## Install and Load Package

If you don’t already have remotes installed, use the code
`install.packages("remotes")`

Then to install this package use the code
`remotes::install_github("ailich/MultiscaleDEM")`

## Main Functions

### Slope, Aspect and Curvature

-   `SlpAsp` calculates multi-scale slope and aspect according to the
    Misiuk et al (2021) which is a modification of the traditional 3 x 3
    slope and aspect algorithms (Fleming and Hoffer, 1979; Horn et al.,
    1981; Ritter, 1987).

-   `WoodEvans` calculates slope, aspect, curvature, and morphometric
    features by fitting a quadratic surface to the focal window using
    ordinary least squares (Wood, 1996). This is similar to
    [r.param.scale in GRASS
    GIS](https://grass.osgeo.org/grass80/manuals/r.param.scale.html).

### Rugosity

-   `VRM` - Vector ruggedness measure (Sappington et al. 2007).

-   `SAPA` - Calculates the Surface Area to Planar Area (Jenness, 2004).
    Additionally, planar area can be corrected for slope (Du Preez
    2015). Additionally, a proposed extension to multiple scales is
    provided by summing the surface areas within the focal window and
    adjusting the planar area of the focal window using multi-scale
    slope.

    -   `SurfaceArea` - Calculate the surface area of each grid cell
        (Jenness, 2004). Used within `SAPA`.

-   `AdjSD`- This new proposed rugosity metric modifies the standard
    deviation of elevation/bathymetry to account for slope. It does this
    by first fitting a plane to the data in the focal window using
    ordinary least squares, and then extracting the residuals, and then
    calculating the standard deviation of the residuals.

![](images/adj_sd.png)

### Relative Position

-   `TPI` - Topographic Position Index (Weiss, 2001)

-   `RDMV` - Relative Difference from Mean Value (Lecours et al., 2017)

## Tutorial

In this tutorial we will calculate various terrain attributes using a 5
x 5 cell rectangular window. Any rectangular odd numbered window size
however could be used. Window sizes are specified with a vector of
length 2 of `c(n_rows, n_cols)`. If a single number is provided it will
be used for both the number of rows and columns.

Load packages

``` r
library(raster) #Load raster package
library(MultiscaleDEM) #Load MultiscaleDEM package
```

See package help page

``` r
help(package="MultiscaleDEM")
```

Read in Data

``` r
r<- raster(volcano, xmn=0, xmx=ncol(volcano)*10, ymn=0, ymx=nrow(volcano)*10) #Use built in volcano data set
crs(r)<- CRS("+proj=utm +zone=16 +datum=WGS84 +units=m +no_defs") #Give r a projection. This is not the right projection, but some functions currently throw an error if there is no crs.
```

![](README_files/figure-gfm/unnamed-chunk-4-1.png)<!-- -->

### Slope, Aspect, and Curvature

``` r
slp_asp<- SlpAsp(r = r, w = c(5,5), unit = "degrees", method = "queen")
```

![](README_files/figure-gfm/unnamed-chunk-6-1.png)<!-- -->

``` r
WE<- WoodEvans(r, w = c(5,5), unit = "degrees", return_aspect = TRUE, na.rm = TRUE, pad = TRUE)
```

![](README_files/figure-gfm/unnamed-chunk-8-1.png)<!-- -->

### Rugosity

``` r
vrm<- VRM(r, w=c(5,5))
```

![](README_files/figure-gfm/unnamed-chunk-10-1.png)<!-- -->

Note: multi-scale SAPA is experimental. The established metric by De
Preez (2015) would use `w=1`.

``` r
sapa<- SAPA(r, w=c(5,5), slope_correction = TRUE)
```

![](README_files/figure-gfm/unnamed-chunk-12-1.png)<!-- -->

``` r
adj_SD<- AdjSD(r, w=c(5,5), na.rm = TRUE, pad=TRUE)
```

![](README_files/figure-gfm/unnamed-chunk-14-1.png)<!-- -->

### Relative Position

``` r
tpi<- TPI(r, w=c(5,5), na.rm = TRUE, pad = TRUE)
```

![](README_files/figure-gfm/unnamed-chunk-16-1.png)<!-- -->

``` r
rdmv<- RDMV(r, w=c(5,5), na.rm = TRUE, pad = TRUE)
```

![](README_files/figure-gfm/unnamed-chunk-18-1.png)<!-- -->

# References

Du Preez, C., 2015. A new arc–chord ratio (ACR) rugosity index for
quantifying three-dimensional landscape structural complexity. Landscape
Ecol 30, 181–192. <https://doi.org/10.1007/s10980-014-0118-8>

Fleming, M.D., Hoffer, R.M., 1979. Machine processing of landsat MSS
data and DMA topographic data for forest cover type mapping (No. LARS
Technical Report 062879). Laboratory for Applications of Remote Sensing,
Purdue University, West Lafayette, Indiana.

Horn, B.K., 1981. Hill Shading and the Reflectance Map. Proceedings of
the IEEE 69, 14–47.

Jenness, J.S., 2004. Calculating landscape surface area from digital
elevation models. Wildlife Society Bulletin 32, 829–839.
<https://doi.org/10.2193/0091-7648(2004)032%5B0829:CLSAFD%5D2.0.CO;2>

Lecours, V., Devillers, R., Simms, A.E., Lucieer, V.L., Brown, C.J.,
2017. Towards a Framework for Terrain Attribute Selection in
Environmental Studies. Environmental Modelling & Software 89, 19–30.
<https://doi.org/10.1016/j.envsoft.2016.11.027>

Misiuk, B., Lecours, V., Dolan, M.F.J., Robert, K., 2021. Evaluating the
Suitability of Multi-Scale Terrain Attribute Calculation Approaches for
Seabed Mapping Applications. Marine Geodesy 44, 327–385.
<https://doi.org/10.1080/01490419.2021.1925789>

Ritter, P., 1987. A vector-based slope and aspect generation algorithm.
Photogrammetric Engineering and Remote Sensing 53, 1109–1111.

Sappington, J.M., Longshore, K.M., Thompson, D.B., 2007. Quantifying
Landscape Ruggedness for Animal Habitat Analysis: A Case Study Using
Bighorn Sheep in the Mojave Desert. The Journal of Wildlife Management
71, 1419–1426. <https://doi.org/10.2193/2005-723>

Weiss, A., 2001. Topographic Position and Landforms Analysis. Presented
at the ESRI user conference, San Diego, CA.

Wood, J., 1996. The geomorphological characterisation of digital
elevation models (Ph.D.). University of Leicester.
