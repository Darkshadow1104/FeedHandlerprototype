GBM.md
Geometric Brownian Motion (Price Simulation)

Overview
Geometric Brownian Motion (GBM) is a stochastic process widely used in finance to model stock prices.
It captures:
•	Random price movement
•	Continuous growth/decay
•	Volatility
GBM is the foundation of:
•	Black-Scholes model
•	Option pricing
•	Market simulation systems

Why GBM?
Real stock prices:
•	Move randomly
•	Show trends (bull/bear)
•	Scale proportionally (₹100 stock vs ₹1000 stock)
GBM models all three:
Randomness + Trend + Scale dependency

Mathematical Model

Core Equation
dS = mu *S * dt + sigma *S * dW

Variables
	
S	Current price
μ (mu)	Drift (trend)
σ (sigma)	Volatility
dt	Time step
dW	Wiener process (random noise)

 Intuition

1. Drift Term
μ S dt
•	Controls the direction of price
•	Positive → bullish
•	Negative → bearish

2. Random Term
σ S dW
•	Introduces randomness
•	Larger σ → more volatility

3. Scale Property
Change ∝ S
₹100 stock moves less than ₹1000 stock

Wiener Process (dW)

Definition
dW ~ N(0, dt)
Normally distributed random variable

Implementation (Box-Muller)
double u1 = rand_uniform();
double u2 = rand_uniform();

double z = sqrt(-2 * log(u1)) * cos(2 * M_PI * u2);
double dW = z * sqrt(dt);

Discrete Implementation

Update Equation
S_{t+1} = S_t + mu* S_t *dt + sigma *S_t *dW

C++ Code
double dW = normal_random() * sqrt(dt);

price += mu * price * dt + sigma * price * dW;

Parameter Selection

Drift (μ)
Market Type	Value
Neutral	0.0
Bullish	+0.05
Bearish	-0.05

Volatility (σ)
Asset Type	Value
Low vol	0.01–0.02
Medium	0.03–0.04
High vol	0.05–0.06

Time Step (dt)
dt = 0.001 (1 ms)
Properties of GBM

1. Log-Normal Distribution
Prices follow:
log(S) ~ Normal distribution

2. No Negative Prices
S > 0 always

3. Continuous Paths
•	Smooth price evolution
•	No sudden jumps (unless an extended model is used)
Limitations

1. No Jumps
Real markets have:
•	news shocks
•	gaps
GBM does NOT model these

2. Constant Volatility
σ is fixed, but real markets:
•	have changing volatility

3. No Market Microstructure
Does not include:
•	order book
•	liquidity
•	bid/ask dynamics

Why GBM in This Project?

GBM is used because:
  Simple to implement
  Computationally cheap
  Produces realistic-looking prices
  Suitable for high-frequency simulation
Example Output Behaviour
Price trajectory:
100 → 101 → 99 → 102 → 103 → ...
•	Random but smooth
•	No extreme jumps
•	Scales with price

Enhancements (Future Work)

1. Jump Diffusion Model
Add sudden jumps:
GBM + Poisson jumps

2. Stochastic Volatility
σ becomes dynamic

3. Mean Reversion
Used for:
•	interest rates
•	commodities

Key Takeaways

•	GBM models random + trend + scale-dependent movement
•	Uses normal distribution for randomness
•	Efficient for simulation systems
•	Forms the basis of modern financial modelling

Conclusion
Geometric Brownian Motion provides a simple yet powerful framework for simulating realistic price movements.
This system enables:
•	independent symbol behaviour
•	configurable volatility
•	scalable tick generation

