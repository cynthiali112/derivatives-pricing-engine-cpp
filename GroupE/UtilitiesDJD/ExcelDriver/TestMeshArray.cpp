#include "ExcelDriverLite.hpp"

#include <iomanip>
#include <iostream>
#include <list>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/math/distributions/normal.hpp>

// Enumeration to represent the type of option (Call or Put).
enum class OptionType
{
	Call,
	Put
};

// Function to calculate the Black-Scholes price of a European option.
double BlackScholesPrice(OptionType type, double T, double K, double sigma, double r, double S, double b)
{
    // Calculate d1 and d2 using the Black-Scholes formula.
	const double d1 = (std::log(S / K) + (b + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
	const double d2 = d1 - sigma * std::sqrt(T);

    // Create a normal distribution object to compute the cumulative distribution function (CDF) values.
	const boost::math::normal_distribution<> normal;

    // Calculate and return the option price based on the type of option (Call or Put).
	if (type == OptionType::Call)
	{
		return S * std::exp((b - r) * T) * cdf(normal, d1)
			- K * std::exp(-r * T) * cdf(normal, d2);
	}

	return K * std::exp(-r * T) * cdf(normal, -d2)
		- S * std::exp((b - r) * T) * cdf(normal, -d1);
}

// Function to create a mesh array from start to end with step size h.
std::vector<double> MeshArray(double start, double end, double h)
{
    // When mesh size h is negative or zero, or when end is less than start, throw an exception.
	if (h <= 0.0)
	{
		throw std::invalid_argument("Mesh size h must be positive.");
	}
	if (end < start)
	{
		throw std::invalid_argument("End value must be greater than or equal to start value.");
	}

    // Create the mesh array.
	std::vector<double> mesh;
    // To avoid floating-point precision issues, we add a small tolerance to the loop condition.
	for (double x = start; x <= end + 1.0e-12; x += h)
	{
        // Add the current value of x to the mesh array.
		mesh.push_back(x);
	}

    // Return the created mesh array.
	return mesh;
}

int main()
{
    // Try block to catch any exceptions that may occur during the execution of the program.
	try
	{
        // Initialize parameters for the European option.
		const double T = 0.25;
		const double K = 65.0;
		const double sigma = 0.30;
		const double r = 0.08;
		const double b = 0.08;

        // Create a mesh array for spot prices from 10 to 50 with a step size of 1.
		const std::vector<double> sMesh = MeshArray(10.0, 50.0, 1.0);

        // Create vectors to hold the computed call and put option prices.
		std::vector<double> callPrices;
		std::vector<double> putPrices;
        // Reserve space in the vectors to improve performance by avoiding multiple reallocations.
		callPrices.reserve(sMesh.size());
		putPrices.reserve(sMesh.size());

        // Loop through each spot price in the mesh array.
		for (double s : sMesh)
		{
            // Compute the call and put option prices for the current asset price and add them to the respective vectors.
			callPrices.push_back(BlackScholesPrice(OptionType::Call, T, K, sigma, r, s, b));
			putPrices.push_back(BlackScholesPrice(OptionType::Put, T, K, sigma, r, s, b));
		}

		std::cout << std::fixed << std::setprecision(6);
		std::cout << "Computed " << sMesh.size() << " option prices over S = 10..50\n";

        // Create an instance of ExcelDriver and make the Excel application visible.
		ExcelDriver xl;
		xl.MakeVisible(true);

        // Create labels for the curves and a list of vectors to hold the call and put prices.
		std::list<std::string> labels;
		labels.push_back("Call");
		labels.push_back("Put");

        // Create a list of vectors to hold the call and put option prices for plotting in Excel.
		std::list<std::vector<double>> curves;
		curves.push_back(callPrices);
		curves.push_back(putPrices);

        // Create a chart in Excel to visualize the option prices against the asset prices.
		xl.CreateChart(sMesh, labels, curves, "European Option Prices vs Spot", "S", "Price");
	}
    // Catch any standard exceptions and print the error message to the console.
	catch (const std::exception& ex)
	{
        // Print the error message to the console if an exception occurs.
		std::cerr << "Error: " << ex.what() << '\n';
		return 1;
	}

	return 0;
}
