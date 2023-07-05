give a m$\times$n image $I$ and noisy image $K$

MSE defined as:

$$
M S E=\frac{1}{m n} \sum_{i=0}^{m-1} \sum_{j=0}^{n-1}[I(i, j)-K(i, j)]^2
$$

then, PSNR is defined as

$$
P S N R=10 \cdot \log _{10}\left(\frac{M A X_I^2}{M S E}\right)
$$

$M A X_I^2$ is the possible maximum pixel value of the image. 255 for uint8 image, 
$M A X_I=2^B-1$ for $B$ bit image.


