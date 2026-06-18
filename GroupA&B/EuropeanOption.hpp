#ifndef EUROPEAN_OPTION_HPP
#define EUROPEAN_OPTION_HPP

// class file for European option, derived from the base option class.

#include <cmath> // for log, sqrt, exp
#include <stdexcept> // for std::invalid_argument

#include <boost/math/distributions/normal.hpp> // for normal distribution and its cdf/pdf

#include "Option.hpp" // Include the base Option class

// EuropeanOption class, implementing the Black-Scholes formula for pricing and Greeks
class EuropeanOption : public Option
{
private:
	// Time to maturity for the option
	double T_;

	static void validateMaturity(double T)
	{
		if (T <= 0.0)
		{
			throw std::invalid_argument("Maturity must be positive for a European option.");
		}
	}

	// Helper functions to calculate d1 and d2 for the Black-Scholes formula
	double d1() const
	{
		// d1 = [ln(S/K) + (b + sigma^2 / 2) * T] / (sigma * sqrt(T))
		return (std::log(S_ / K_) + (b_ + 0.5 * sigma_ * sigma_) * T_) / (sigma_ * std::sqrt(T_));
	}

	double d2() const
	{
		// d2 = d1 - sigma * sqrt(T)
		return d1() - sigma_ * std::sqrt(T_);
	}

public:
	// Constructors for the EuropeanOption class, with and without cost of carry
	// The first constructor assumes cost of carry equals risk-free rate (b = r)
	EuropeanOption(OptionType type, double T, double K, double sigma, double r, double S)
		: Option(type, K, sigma, r, S, r), T_(T) // Initialize T_ and set b = r in the base class constructor
	{
		// Validate the maturity T to ensure it's positive, as required for a European option
		validateMaturity(T_);
	}
	
	// The second constructor allows specifying a different cost of carry b to support more general cases
	EuropeanOption(OptionType type, double T, double K, double sigma, double r, double S, double b)
		: Option(type, K, sigma, r, S, b), T_(T) // Initialize T_ and set b in the base class constructor
	{
		// Validate the maturity T to ensure it's positive, as required for a European option
		validateMaturity(T_);
	}

	// Override the price function to implement the Black-Scholes formula for European options
	double price() const override
	{
		// Use the normal distribution from Boost to calculate the cumulative distribution function (CDF) and probability density function (PDF)
		const boost::math::normal_distribution<> normal;
		const double d1Value = d1(); // Calculate d1 using the helper function
		const double d2Value = d2(); // Calculate d2 using the helper function

		// Calculate the option price based on whether it's a call or put option
		if (type_ == OptionType::Call)
		{
			// Call option price = S * exp((b - r) * T) * N(d1) - K * exp(-r * T) * N(d2)
			return (S_ * std::exp((b_ - r_) * T_) * cdf(normal, d1Value)) - (K_ * std::exp(-r_ * T_) * cdf(normal, d2Value));
		}
		// Put option price = K * exp(-r * T) * N(-d2) - S * exp((b - r) * T) * N(-d1)
		return (K_ * std::exp(-r_ * T_) * cdf(normal, -d2Value)) - (S_ * std::exp((b_ - r_) * T_) * cdf(normal, -d1Value));
	}

	// Implement the delta and gamma functions to calculate the Greeks for the European option
	double delta() const // Calculate the delta of the option
	{
		const boost::math::normal_distribution<> normal; // Standard normal distribution for CDF and PDF calculations
		const double d1Value = d1(); // Calculate d1 using the helper function

		// Test if it's a call or put option and calculate delta accordingly
		if (type_ == OptionType::Call)
		{
			// Call option delta = exp((b - r) * T) * N(d1)
			return std::exp((b_ - r_) * T_) * cdf(normal, d1Value);
		}

		// Put option delta = exp((b - r) * T) * (N(d1) - 1)
		return std::exp((b_ - r_) * T_) * (cdf(normal, d1Value) - 1.0);
	}

	double gamma() const // Calculate the gamma of the option
	{
		const boost::math::normal_distribution<> normal; // Standard normal distribution for CDF and PDF calculations
		const double d1Value = d1(); // Calculate d1 using the helper function
		// Gamma is the same for both call and put options, so we can calculate it directly
		// Gamma = [exp((b - r) * T) * N'(d1)] / (S * sigma * sqrt(T))
		return (std::exp((b_ - r_) * T_) * pdf(normal, d1Value)) / (S_ * sigma_ * std::sqrt(T_));
	}
};

#endif
