#ifndef COMMON_UTILITIES_HPP
#define COMMON_UTILITIES_HPP

// Common utility functions and data structures for option pricing, including input generation, mesh array creation, and matrix evaluation.

#include <functional> // for std::function
#include <iomanip> // for std::setw
#include <iostream> // for std::cout
#include <sstream> // for std::ostringstream
#include <string> // for std::string
#include <vector> // for std::vector

// Structure to hold the input parameters for an option, including a name for identification and all necessary parameters for pricing
// Ensure easy access to option parameters in a structured way for batch processing and matrix evaluations
struct OptionInput
{
	std::string name; // A descriptive name for the option input, useful for identifying batches and test cases
	double T; // Time to maturity
	double K; // Strike price
	double sigma; // Volatility
	double r; // Risk-free rate
	double S; // Asset price
	double b; // Cost of carry
};

// Function to create a mesh array of values from start to end with a step size h, used for generating asset price ranges for pricing and Greeks calculations
inline std::vector<double> MeshArray(double start, double end, double h)
{
    // Create a vector to hold the generated values
	std::vector<double> values;

    // Validate the input parameters to ensure a valid array can be created
	if (h <= 0.0 || end < start) // if step size is non-positive or end is less than start
	{
		return values; // Return an empty vector for invalid input parameters
	}

    // Generate values from start to end with step size h, including end if it falls within the range
	for (double x = start; x <= end + 0.5 * h; x += h)
	{
        // Add the generated value to the vector
		values.push_back(x);
	}

    // Return the generated vector of values
	return values;
}

using InputRow = std::vector<OptionInput>; // A row of option inputs, useful for batch processing
using InputMatrix = std::vector<InputRow>; // A matrix of option inputs, useful for evaluating multiple batches
using PriceFunctor = std::function<double(const OptionInput&)>; // A functor for evaluating the price of an option

// Option parameter enumeration for identifying which parameter to vary in matrix evaluations
enum class OptionParameter
{
	T,
	K,
	Sigma,
	R,
	S,
	B
};

// Utility function to format a parameter value as a string with fixed precision, used for labeling matrix entries
inline std::string formatParameterValue(double value)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(4) << value; // Format the parameter value with fixed-point notation and 4 decimal places for consistent labeling in output
	return oss.str(); // Return the formatted parameter value as a string
}

// Function to create a new OptionInput with one parameter value changed, used for generating input matrices by varying a specific parameter
inline OptionInput withParameterValue(const OptionInput& input, OptionParameter parameter, double value)
{
	OptionInput output = input; // Start with a copy of the original input and modify the specified parameter to create a new input for matrix evaluations

	switch (parameter) // Update the specified parameter in the output OptionInput based on the provided enumeration value
	{
	case OptionParameter::T:
		output.T = value;
		break;
	case OptionParameter::K:
		output.K = value;
		break;
	case OptionParameter::Sigma:
		output.sigma = value;
		break;
	case OptionParameter::R:
		output.r = value;
		break;
	case OptionParameter::S:
		output.S = value;
		break;
	case OptionParameter::B:
		output.b = value;
		break;
	}

	return output; // Return the modified OptionInput with the updated parameter value
}

// Function to build a matrix where each sweep value becomes its own row with a single OptionInput.
// baseInput serves as the template for all rows, and the specified parameter is varied across the sweep values to create a matrix of inputs for pricing evaluations.
// sweepValues is the vector of values to vary the specified parameter across
// parameterLabel is used for labeling the name of each input for identification in output.
inline InputMatrix buildInputMatrixFromSweep(const OptionInput& baseInput,
	const std::vector<double>& sweepValues,
	OptionParameter parameter,
	const std::string& parameterLabel)
{
	// Create a matrix to hold the generated input rows, reserving space based on the size of the sweep values for efficiency
	InputMatrix matrix;
	matrix.reserve(sweepValues.size());

	// Iterate through each value in the sweep values, create a new OptionInput with the specified parameter varied, and add it as a new row in the matrix
	for (double value : sweepValues)
	{
		// Create a new OptionInput with the specified parameter varied using the withParameterValue function, and label it for identification in output
		OptionInput varied = withParameterValue(baseInput, parameter, value);
		// If a parameter label is provided, append it to the name of the input for better identification in output
		if (!parameterLabel.empty())
		{
			varied.name = baseInput.name + " | " + parameterLabel + "=" + formatParameterValue(value);
		}
		// Add the new OptionInput as a new row in the matrix 
		matrix.push_back(InputRow{varied}); // one input per row
	}

	// Return the generated matrix
	return matrix;
}

// Function to evaluate a matrix of option inputs using a provided pricing functor, returning a matrix of calculated prices
inline std::vector<std::vector<double>> evaluateMatrix(const InputMatrix& matrix, const PriceFunctor& evaluator)
{
	// Create a result matrix to hold the evaluated prices, reserving space for efficiency
	std::vector<std::vector<double>> result;
	// Reserve space for the result matrix based on the size of the input matrix to improve performance
	result.reserve(matrix.size());

	// Iterate through each row of the input matrix
	for (const auto& row : matrix)
	{
		// Create a row for the evaluated prices, reserving space based on the size of the input row
		std::vector<double> valueRow;
		valueRow.reserve(row.size());
		// Iterate through each option input in the row and evaluate the price using the provided functor
		for (const auto& input : row)
		{
			// Evaluate the price using the provided functor and add it to the value row
			valueRow.push_back(evaluator(input));
		}
		// Add the evaluated price row to the result matrix
		result.push_back(valueRow);
	}

	return result;
}

// Print utility functions for formatting output

// Function to print a separator line for better readability of output sections
inline void printLine()
{
	std::cout << "------------------------------------------------------------\n";
}

// Function to print a series of values corresponding to a series of asset prices, with a label for identification
inline void printSeries(const std::vector<double>& assetArray, const std::vector<double>& valueArray, const std::string& label)
{
	// Print the label for the series
	std::cout << label << "\n";
	// Iterate through the asset prices and corresponding values, printing them in a formatted manner
	// Ensure that we do not access out of bounds by checking the size of both vectors
	for (std::size_t i = 0; i < assetArray.size() && i < valueArray.size(); ++i)
	{
		// Print the asset price and corresponding value with formatting for better readability
		std::cout << "S=" << std::setw(6) << assetArray[i] << " --> " << std::setw(12) << valueArray[i] << "\n";
	}
}

// Function to print a matrix of values with a title, formatting the output for better readability
inline void printMatrix(const std::vector<std::vector<double>>& matrix, const std::string& title)
{
	// Print the title for the matrix
	std::cout << title << "\n";
	// Iterate through each row of the matrix and print the values in a formatted manner
	for (std::size_t i = 0; i < matrix.size(); ++i)
	{
		// Print the row number for identification
		std::cout << "Row " << i + 1 << ": ";
		// Iterate through each value in the row and print it with formatting
		for (double value : matrix[i])
		{
			// Print the value with formatting for
			std::cout << std::setw(12) << value << ' ';
		}
		// Print a newline after each row
		std::cout << "\n";
	}
}

#endif
