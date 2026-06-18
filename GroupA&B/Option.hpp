#ifndef OPTION_HPP
#define OPTION_HPP

#include <stdexcept>

// OptionType enumeration to distinguish between call and put options
enum class OptionType
{
	Call, 
	Put
};

// Abstract base class for options
class Option
{
// Common inputs for all options, protected members to be used by derived classes
protected:
	OptionType type_; // Call or Put
	double K_; // Strike price
	double sigma_; // Volatility
	double r_; // Risk-free rate
	double S_; // Asset price
	double b_; // Cost of carry

	// Validate common inputs for options
	static void validateCommonInputs(double S, double K, double sigma)
	{
		// Basic validation for option parameters
		if (S <= 0.0 || K <= 0.0) // if asset or strike price is non-positive, it's invalid
		{
			throw std::invalid_argument("Spot and strike must be positive.");
		}
		if (sigma <= 0.0) // if volatility is non-positive, it's invalid
		{
			throw std::invalid_argument("Volatility must be positive.");
		}
	}

public:
	// Constructors for the Option class, with and without cost of carry
	// The first constructor assumes cost of carry equals risk-free rate (b = r)
	Option(OptionType type, double K, double sigma, double r, double S)
		: type_(type), K_(K), sigma_(sigma), r_(r), S_(S), b_(r)
	{
		// Validate common inputs for the option parameters
		validateCommonInputs(S_, K_, sigma_);
	}

	// The second constructor allows specifying a different cost of carry b to support more general cases 
	Option(OptionType type, double K, double sigma, double r, double S, double b)
		: type_(type), K_(K), sigma_(sigma), r_(r), S_(S), b_(b)
	{
		// Validate common inputs for the option parameters
		validateCommonInputs(S_, K_, sigma_);
	}

	// Virtual destructor to ensure proper cleanup of derived classes
	virtual ~Option() = default;

	// Pure virtual function to be implemented by derived classes to calculate the option price
	virtual double price() const = 0;

};

#endif

