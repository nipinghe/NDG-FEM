#Idealized dam break in a rectangular channel

#1.Governing Equations
$$\frac{\partial U}{\partial t} + \frac{\partial F}{\partial x} = 0$$
$$U = \begin{bmatrix} h \cr q \end{bmatrix} \quad
F = \begin{bmatrix} q \cr gh^2/2 + q^2/h \end{bmatrix}$$

#2.Discrete with DGM

$$U_h = \sum{l_j U_j} \quad F_h(U) = \sum{l_j F(U_j)}$$
$$\int_{\Omega} l_i l_j \frac{\partial U_j}{\partial t} dx+ 
\int_{\Omega} l_i \frac{\partial l_j}{\partial x} F(U_j) dx= 0$$

$$\int_{\Omega} l_i l_j \frac{\partial U_j}{\partial t} dx + 
\int_{\Omega} l_i \frac{\partial l_j}{\partial x} F(U_j) dx+
\oint_{\partial \Omega} l_i l_j (F^* - F)\cdot \vec{n} ds = 0 $$

$$JM \frac{\partial U}{\partial t} + JMD_x F(U) + J_E M_E (F^* - F)\cdot \vec{n} = 0$$

ODE:
$$\frac{\partial U}{\partial t} = -\frac{\partial r}{\partial x}D_r F(U) + \frac{J_E}{J}M^{-1} M_E (F^* - F)\cdot \vec{n}=L(U(t))$$

$$rhs = -\frac{\partial r}{\partial x}D_r F(U) + \frac{J_E}{J}M^{-1} M_E (F - F^*)\cdot \vec{n}$$

#3.Numerical Flux
##3.1.HLL flux

$$F^{HLL} = \left\\{
\begin{matrix}
F^- \cr
\frac{S_R F^- - S_L F^+ + S_L S_R(U^+ - U^-)}{S_R S_L} \cr
F^+ \end{matrix} \right. 
\begin{matrix}
S_L \geq 0 \cr
S_L < 0 < S_R \cr
S_R \leq 0
\end{matrix}$$

Wave Speed is suggested by Fraccarollo and Toro (1995)

$$ S_L = min(u^- - \sqrt{gh^-}, u^\* - c^*)$$

$$ S_R = min(u^+ + \sqrt{gh^+}, u^\* + c^*)$$

$u^\*$ and $c^\*$ is defined by

$$u^\* = \frac{1}{2}(u^- + u^+) + \sqrt{gh^-} - \sqrt{gh^+}$$

$$c^\* = \frac{1}{2}(\sqrt{gh^-} + \sqrt{gh^+}) + \frac{1}{4}(u^- - u^+)$$

##3.2.Coordinate Transformation
If $F' = F\cdot \frac{\partial x'}{\partial x} = -F$, then derived corresponding Jacobian Matrix 
$$A' = \partial F'/ \partial U = -A$$
 
The original Jacobian Matrix $A$ and its eigenvalues is

$$A = \begin{bmatrix} 
0 & 1 \cr
c^2 - u^2 & 2u \cr
\end{bmatrix}$$

$$\lambda_1 = u-c \quad \lambda_2 = u+c$$

$A'$ and its eigenvalues is

$$A = \begin{bmatrix} 
0 & -1 \cr
u^2 - c^2 & -2u \cr
\end{bmatrix}$$

$$\lambda'_1 =-u-c \quad \lambda'_2 = -u+c$$

**After the transformation of coordinate, the wave speed is not the negative of original wave speed.**

#4.Limiter

minmod limiter

[TBC]

#5.Numerical Test

Ideal dam break

| | |
| --- | --- |
| channel length | 1000m |
| dam position | 500m |
| upstream h | 10m |
| downstream h | 2m |
| element num | 400 |
| Final Time | 20s |

![](../fig/depth.png)
![](../fig/flux.png)



