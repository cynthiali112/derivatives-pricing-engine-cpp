#ifndef EUROPEAN_UTILITIES_HPP
#define EUROPEAN_UTILITIES_HPP

// Utility functions for European options, including batch input generation, parity checks, and price/Greek calculations across a range of asset prices.

#include <cmath> // for exp, fabs
#include <vector> // for std::vector

#include "CommonUtilities.hpp"
#include "EuropeanOption.hpp"

// Function to generate batches of input parameters for European option pricing, useful for testing and validation against known results
inline std::vector<OptionInput> europeanBatches()
{
	// Return a vector of OptionInput structures
	return {
		{"Batch New", 1.45, 120.0, 0.51, 0.045, 108.0, 0.0}, // T, K, sigma, r, S, b
		{"Batch 2", 1.00, 100.0, 0.20, 0.00, 100.0, 0.00},
		{"Batch 3", 1.00, 10.0, 0.50, 0.12, 5.0, 0.12},
		{"Batch 4", 30.0, 100.0, 0.30, 0.08, 100.0, 0.08},
	};
}

// Function to calculate the put price from a given call price using put-call parity, useful for validating pricing consistency
inline double parityPutFromCall(double callPrice, double S, double K, double r, double T)
{
	// P = C + K*exp(-rT) - S
	return callPrice + K * std::exp(-r * T) - S;
}

// Function to calculate the call price from a given put price using put-call parity, useful for validating pricing consistency
inline double parityCallFromPut(double putPrice, double S, double K, double r, double T)
{
	// C = P + S - K*exp(-rT)
	return putPrice + S - K * std::exp(-r * T);
}

// Function to check if the put-call parity holds for given call and put prices, returning true if the parity condition is satisfied within a specified tolerance
inline bool satisfiesParity(double callPrice, double putPrice, double S, double K, double r, double T, double tolerance = 1e-8)
{
	// left side of parity: C + K*exp(-rT)
	const double left = callPrice + K * std::exp(-r * T);
	// right side of parity: P + S
	const double right = putPrice + S;
	// Check if the absolute difference between the left and right sides is within the specified tolerance
	return std::fabs(left - right) <= tolerance;
}

// Function to calculate European option prices across a range of asset prices
inline std::vector<double> europeanPriceAcrossS(const OptionInput& input, OptionType type, const std::vector<double>& assetArray)
{
	// Create a vector to hold the calculated prices, reserving space based on the size of the input asset price vector
	std::vector<double> priceArray;
	priceArray.reserve(assetArray.size());
	
	// Iterate through each asset price in the input vector, create a EuropeanOption for each price, and calculate the option price, adding it to the prices vector
	for (double asset : assetArray)
	{
		// Create a EuropeanOption instance for the current asset price
		EuropeanOption option(type, input.T, input.K, input.sigma, input.r, asset, input.b);
		priceArray.push_back(option.price()); // Calculate the option price using the price() method and add it to the price array
	}

	// Return the vector of calculated option prices across the range of asset prices
	return priceArray;
}

// Function to calculate the delta of European options across a range of asset prices
inline std::vector<double> europeanDeltaAcrossS(const OptionInput& input, OptionType type, const std::vector<double>& assetArray)
{
	// Create a vector to hold the calculated deltas, reserving space based on the size of the input asset price vector
	std::vector<double> deltaArray;
	deltaArray.reserve(assetArray.size());

	// Iterate through each asset price in the input vector, create a EuropeanOption for each price, and calculate the option delta, adding it to the deltas vector
	for (double asset : assetArray)
	{
		// create a EuropeanOption instance for the current asset price
		EuropeanOption option(type, input.T, input.K, input.sigma, input.r, asset, input.b);
		deltaArray.push_back(option.delta()); // Calculate the option delta using the delta() method and add it to the delta array
	}

	// Return the vector of calculated option deltas across the range of asset prices
	return deltaArray;
}

// Function to calculate the gamma of European options across a range of asset prices
inline std::vector<double> europeanGammaAcrossS(const OptionInput& input, const std::vector<double>& assetArray)
{
	// Create a vector to hold the calculated gammas, reserving space based on the size of the input asset price vector
	std::vector<double> gammaArray;
	gammaArray.reserve(assetArray.size());

	// Iterate through each asset price in the input vector, create a EuropeanOption for each price, and calculate the option gamma, adding it to the gammas vector
	for (double asset : assetArray)
	{
		// create a EuropeanOption instance for the current asset price
		EuropeanOption callOption(OptionType::Call, input.T, input.K, input.sigma, input.r, asset, input.b);
		gammaArray.push_back(callOption.gamma()); // Calculate the option gamma using the gamma() method and add it to the gamma array
	}

	// Return the vector of calculated option gammas across the range of asset prices
	return gammaArray;
}

// Function to calculate the delta of a European option using finite difference approximation
inline double finiteDifferenceDelta(const OptionInput& input, OptionType type, double h) // h is the step size for finite difference approximation
{
	// Upper asset price for finite difference calculation
	EuropeanOption up(type, input.T, input.K, input.sigma, input.r, input.S + h, input.b);
	// Lower asset price for finite difference calculation
	EuropeanOption down(type, input.T, input.K, input.sigma, input.r, input.S - h, input.b);
	// Calculate the finite difference approximation of delta: (V(S+h) - V(S-h)) / (2h)
	return (up.price() - down.price()) / (2.0 * h);
}

// Function to calculate the gamma of a European option using finite difference approximation
inline double finiteDifferenceGamma(const OptionInput& input, OptionType type, double h) // h is the step size for finite difference approximation
{
	// Upper asset price for finite difference calculation
	EuropeanOption up(type, input.T, input.K, input.sigma, input.r, input.S + h, input.b);
	// Middle asset price for finite difference calculation
	EuropeanOption mid(type, input.T, input.K, input.sigma, input.r, input.S, input.b);
	// Lower asset price for finite difference calculation
	EuropeanOption down(type, input.T, input.K, input.sigma, input.r, input.S - h, input.b);
	// Calculate the finite difference approximation of gamma: (V(S+h) - 2V(S) + V(S-h)) / (h^2)
	return (up.price() - 2.0 * mid.price() + down.price()) / (h * h);
}

#endif
