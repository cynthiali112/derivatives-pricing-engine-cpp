#ifndef PERPETUAL_AMERICAN_UTILITIES_HPP
#define PERPETUAL_AMERICAN_UTILITIES_HPP

#include <vector>

// Utility functions for Perpetual American options, including price calculations across a range of asset prices.

#include "CommonUtilities.hpp"
#include "PerpetualAmericanOption.hpp"

// Function to calculate the price of a perpetual American option given the input parameters and option type
inline double perpetualPrice(const OptionInput& input, OptionType type)
{
	// Create a PerpetualAmericanOption instance using the input parameters and specified option type
	PerpetualAmericanOption option(type, input.K, input.sigma, input.r, input.S, input.b);
	// Calculate and return the option price using the price() method of the PerpetualAmericanOption class
	return option.price();
}

// Function to calculate perpetual American option prices across a range of asset prices
inline std::vector<double> perpetualPriceAcrossS(const OptionInput& input, OptionType type, const std::vector<double>& assetArray)
{
	// Create a vector to hold the calculated prices, reserving space based on the size of the input asset price vector
	std::vector<double> priceArray;
	priceArray.reserve(assetArray.size());

	// Iterate through each asset price in the input vector
	for (double asset : assetArray)
	{
		// Create a PerpetualAmericanOption instance for the current asset price
		PerpetualAmericanOption option(type, input.K, input.sigma, input.r, asset, input.b);
		// Calculate the option price and add it to the price array
		priceArray.push_back(option.price());
	}

	// Return the vector of calculated option prices across the range of asset prices
	return priceArray;
}

#endif
