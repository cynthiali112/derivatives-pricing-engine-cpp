// HardCoded.cpp
//
// C++ code to price an option, essential algorithms.
//
// We take CEV model with a choice of the elaticity parameter
// and the Euler method. We give option price and number of times
// S hits the origin.
//
// (C) Datasim Education BC 2008-2011
//

#include "OptionData.hpp" 
#include "UtilitiesDJD/RNG/NormalGenerator.hpp"
#include "UtilitiesDJD/Geometry/Range.cpp"
#include <cmath>
#include <iostream>
#include <vector>

template <class T> void print(const std::vector<T>& myList)
{  // A generic print function for vectors
	
	std::cout << std::endl << "Size of vector is " << myList.size() << "\n[";

	// We must use a const iterator here, otherwise we get a compiler error.
	typename std::vector<T>::const_iterator i;
	for (i = myList.begin(); i != myList.end(); ++i)
	{
			std::cout << *i << ",";

	}

	std::cout << "]\n";
}

// Standard deviation function: calculates the standard deviation of the discounted payoffs from the Monte Carlo simulation
template <typename T>
double standardDeviation(const std::vector<T>& payoffs, double r, double expiry)
{
	// M=number of simulations, must be at least 2 to calculate standard deviation
	const std::size_t M = payoffs.size();
	if (M < 2)
	{
		return 0.0;
	}

	// Calculate the sum of payoffs and the sum of squares of payoffs to compute the variance
	double sum = 0.0;
	double sumSquares = 0.0;

	// Loop through each payoff value, convert it to double, and accumulate the sum and sum of squares
	for (const auto& value : payoffs)
	{
		const double c = static_cast<double>(value);
		sum += c;
		sumSquares += c * c;
	}

	// Calculate the variance using the formula: variance = (sum of squares - (sum^2 / M)) / (M - 1)
	const double variance = (sumSquares - (sum * sum) / static_cast<double>(M)) /
		static_cast<double>(M - 1);
	// Return the standard deviation, which is the square root of the variance, discounted back to present value using exp(-r * expiry)
	return std::sqrt(variance) * std::exp(-r * expiry);
}

// Standard error function: calculates the standard error of the discounted payoffs from the Monte Carlo simulation
template <typename T>
double standardError(const std::vector<T>& payoffs, double r, double expiry)
{
	// M=number of simulations
	const std::size_t M = payoffs.size();
	// If there are no simulations, return 0 to avoid division by zero
	if (M == 0)
	{
		return 0.0;
	}

	// The standard error is the standard deviation divided by the square root of the number of simulations, discounted back to present value using exp(-r * expiry)
	return standardDeviation(payoffs, r, expiry) / std::sqrt(static_cast<double>(M));
}

namespace SDEDefinition
{ // Defines drift + diffusion + data

	OptionData* data;				// The data for the option MC

	double drift(double t, double X)
	{ // Drift term
	
		return (data->r)*X; // r - D
	}

	
	double diffusion(double t, double X)
	{ // Diffusion term
	
		double betaCEV = 1.0;
		return data->sig * pow(X, betaCEV);
		
	}

	double diffusionDerivative(double t, double X)
	{ // Diffusion term, needed for the Milstein method
	
		double betaCEV = 1.0;
		return 0.5 * (data->sig) * (betaCEV) * pow(X, 2.0 * betaCEV - 1.0);
	}
} // End of namespace


int main()
{
	std::cout << "1 factor MC with explicit Euler\n";

	long N = 100;
	std::cout << "Number of subintervals in time: ";
	std::cin >> N;

	// V2 mediator stuff
	long NSim = 50000;
	std::cout << "Number of simulations: ";
	std::cin >> NSim;

	// Create BatchData struct to hold the input parameters and exact prices for each batch of option data
	struct BatchData
	{
		double T;
		double K;
		double sig;
		double r;
		double S0;
		double exactC;
		double exactP;
	};

	// Inttialize the batches with the specified parameters and exact call and put prices for each batch
	std::vector<BatchData> batches;
	batches.push_back({0.25, 65.0, 0.30, 0.08, 60.0, 2.13337, 5.84628}); // T, K, sigma, r, S0, exact call price, exact put price
	batches.push_back({1.00, 100.0, 0.20, 0.00, 100.0, 7.96557, 7.96557});
	batches.push_back({1.28, 155.0, 0.27, 0.04, 150.0, 0.0, 0.0});
	// Iterate through each batch of option data
	for (std::size_t b = 0; b < batches.size(); ++b)
	{
		// Create an OptionData instance and populate it with the parameters from the current batch
		OptionData myOption;
		myOption.K = batches[b].K;
		myOption.T = batches[b].T;
		myOption.r = batches[b].r;
		myOption.sig = batches[b].sig;
		myOption.type = +1;	// Put -1, Call +1
		double S_0 = batches[b].S0;

		// Print the batch parameters and exact call and put prices for verification
		std::cout << "\nBatch " << (b + 1)
				  << ": T=" << myOption.T
				  << ", K=" << myOption.K
				  << ", sig=" << myOption.sig
				  << ", r=" << myOption.r
				  << ", S=" << S_0 << std::endl;
		std::cout << "Exact Call: " << batches[b].exactC
				  << ", Exact Put: " << batches[b].exactP << std::endl;

		// Create the basic SDE (Context class)
		Range<double> range (0.0, myOption.T);
		double VOld = S_0;
		double VNew;

		std::vector<double> x = range.mesh(N);

		double k = myOption.T / double (N);
		double sqrk = sqrt(k);

		// Normal random number
		double dW;
		double price = 0.0;	// Option price

		// NormalGenerator is a base class
		BoostNormal myNormal;

		using namespace SDEDefinition;
		SDEDefinition::data = &myOption;

		std::vector<double> res;
		int coun = 0; // Number of times S hits origin

		// A.
		for (long i = 1; i <= NSim; ++i)
		{ // Calculate a path at each iteration
				
			if ((i/10000) * 10000 == i)
			{// Give status after each 1000th iteration

					std::cout << i << std::endl;
			}

			VOld = S_0;
			for (unsigned long index = 1; index < x.size(); ++index)
			{

				// Create a random number
				dW = myNormal.getNormal();
					
				// The FDM (in this case explicit Euler)
				VNew = VOld  + (k * drift(x[index-1], VOld))
							+ (sqrk * diffusion(x[index-1], VOld) * dW);

				VOld = VNew;

				// Spurious values
				if (VNew <= 0.0) coun++;
			}

			double tmp = myOption.myPayOffFunction(VNew);
			res.push_back(tmp);
			price += (tmp)/double(NSim);
		}
		
		// D. Finally, discounting the average price
		price *= exp(-myOption.r * myOption.T);

		// Print result
		std::cout << "Price, after discounting: " << price << ", " << std::endl;
		std::cout << "Standard deviation: " << standardDeviation(res, myOption.r, myOption.T) << std::endl;
		std::cout << "Standard error: " << standardError(res, myOption.r, myOption.T) << std::endl;
		std::cout << "Absolute error vs exact put: " << std::fabs(price - batches[b].exactP) << std::endl;
		std::cout << "Number of times origin is hit: " << coun << endl;
	}

	return 0;
}