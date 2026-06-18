#ifndef PERPETUAL_AMERICAN_OPTION_HPP
#define PERPETUAL_AMERICAN_OPTION_HPP

// Class file for Perpetual American option, derived from the base option class.

#include <cmath>
#include <stdexcept>

#include "Option.hpp"

class PerpetualAmericanOption : public Option
{
public:
	// Constructors for the PerpetualAmericanOption class, with and without cost of carry
	// The first constructor assumes cost of carry equals risk-free rate (b = r)
	PerpetualAmericanOption(OptionType type, double K, double sigma, double r, double S)
		: Option(type, K, sigma, r, S, r) // Initialize b = r in the base class constructor
	{
		// Validate that the risk-free rate is positive, as required for perpetual closed-form pricing
		if (r_ <= 0.0)
		{
			throw std::invalid_argument("Risk-free rate must be positive for perpetual closed-form pricing.");
		}
	}

	// The second constructor allows specifying a different cost of carry b to support more general cases
	PerpetualAmericanOption(OptionType type, double K, double sigma, double r, double S, double b)
		: Option(type, K, sigma, r, S, b) // Initialize perpetual American option with the option base class constructor
	{
		// Validate that the risk-free rate is positive, as required for perpetual closed-form pricing
		if (r_ <= 0.0)
		{
			throw std::invalid_argument("Risk-free rate must be positive for perpetual closed-form pricing.");
		}
	}

	// Override the price function to implement the closed-form solution for perpetual American options
	double price() const override
	{
		constexpr double eps = 1e-12; // A small epsilon value to guard against numerical instability in the pricing formula

		// Initialize some common terms to simplify the pricing formula
		const double sigma2 = sigma_ * sigma_; // sigma^2
		const double fac = 0.5 - b_ / sigma2; // A common term in the exponent calculations

		if (type_ == OptionType::Call)
		{
			// Calculate y1 for the call option pricing formula
			const double y1 = fac + std::sqrt(fac * fac + 2.0 * r_ / sigma2);

			// Guard against near-singularity in denominator (y1 - 1).
			if (std::fabs(y1 - 1.0) < eps)
			{
				// In this case, the option price simplifies to the asset price S.
				return S_;
			}

			// Guard against near-singularity in denominator y1.
			if (std::fabs(y1) < eps)
			{
				// In this case, the formula breaks numerically.
				throw std::overflow_error("Perpetual call formula is unstable: denominator (y1) is near 0.");
			}

			const double fac2 = ((y1 - 1.0) * S_) / (y1 * K_); // intermediate factor for the call option price calculation
			return K_ * std::pow(fac2, y1) / (y1 - 1.0); // Final call option price using the closed-form formula for perpetual American options
		}

		// if the option is a put, calculate y2 for the put option pricing formula
		const double y2 = fac - std::sqrt(fac * fac + 2.0 * r_ / sigma2);

		// Guard against near-singularity in denominator (y2 - 1).
		if (std::fabs(y2-1) < eps)
		{
			// In this case, the formula breaks numerically.
			throw std::overflow_error("Perpetual put formula is unstable: denominator (y2-1) is near 0.");
		}
		// Guard against near-singularity in denominator y2.
		if (std::fabs(y2) < eps)
		{
			// In this case, the option price simplifies to the strike price K
			return K_;
		}

		const double fac2 = ((y2 - 1.0) * S_) / (y2 * K_); // intermediate factor for the put option price calculation
		return K_ * std::pow(fac2, y2) / (1.0 - y2); // Final put option price using the closed-form formula for perpetual American options
	}
};

#endif
