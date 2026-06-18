// Test program for Group A&B
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

#include "CommonUtilities.hpp"
#include "EuropeanUtilities.hpp"
#include "PerpetualAmericanUtilities.hpp"

int main()
{
	std::cout << std::fixed << std::setprecision(6); // Set fixed-point notation (6 decimal places) for consistent output formatting

	// A: Exact Solutions of One-Factor Plain Options

	std::cout << "A: Exact Solutions of One-Factor Plain Options\n";

	// A.a) Call and put option pricing using the data sets Batch 1 to Batch 4.
	printLine();
	std::cout << "A.a) European Option Exact Pricing (Batches 1-4)\n";
	printLine();

	// Intialize the batches
	const auto batchSets = europeanBatches();

	// Iterate through each batch of input
	for (std::size_t i = 0; i < batchSets.size(); ++i) 
	{
		// Extract the batch input parameters
		const auto& batch = batchSets[i];
		// Create EuropeanOption instances for both call and put options using the batch parameters
		EuropeanOption call(OptionType::Call, batch.T, batch.K, batch.sigma, batch.r, batch.S, batch.b);
		EuropeanOption put(OptionType::Put, batch.T, batch.K, batch.sigma, batch.r, batch.S, batch.b);

		// Calculate the call and put option prices using the price() method of the EuropeanOption class
		const double callPrice = call.price();
		const double putPrice = put.price();

		// Print the batch name along with the calculated call and put prices
		std::cout << batch.name
			<< " C = " << callPrice
			<< " P = " << putPrice << "\n";
	}

	// A.b) Apply the put-call parity relationship to compute call and put option prices.
	// First approach: calculate the call (or put) price for a corresponding put (or call) price,
	// Second approach: check if a given set of put/call prices satisfy parity

	printLine();
	std::cout << "A.b) Put-Call Parity Conversions and Checks\n";
	printLine();

	// Iterate through each batch of input and perform put-call parity calculations and checks
	for (const auto& batch : batchSets)
	{
		// Create EuropeanOption instances for both call and put options using the batch parameters
		EuropeanOption call(OptionType::Call, batch.T, batch.K, batch.sigma, batch.r, batch.S, batch.b);
		EuropeanOption put(OptionType::Put, batch.T, batch.K, batch.sigma, batch.r, batch.S, batch.b);

		// Calculate the call and put option prices using the price() method of the EuropeanOption class
		const double callPrice = call.price();
		const double putPrice = put.price();
		// Calculate the put price from the call price and the call price from the put price using the parity functions
		const double parityPut = parityPutFromCall(callPrice, batch.S, batch.K, batch.r, batch.T);
		const double parityCall = parityCallFromPut(putPrice, batch.S, batch.K, batch.r, batch.T);
		
		// Print the batch name along with the calculated call and put prices, the parity-derived prices, and whether the parity condition is satisfied
		std::cout << batch.name
			<< " PutFromCall = " << parityPut
			<< " CallFromPut = " << parityCall
			// Use ?: operator to print whether the parity condition is satisfied based on the satisfiesParity function
			<< " Parity = " << (satisfiesParity(callPrice, putPrice, batch.S, batch.K, batch.r, batch.T) ? "Satisfied" : "Failed")
			<< "\n";
	}

	// A.c) Compute option prices for a monotonically increasing range of underlying values of S, for example 10, 11, 12, …, 50

	printLine();
	std::cout << "A.c) Vector Pricing Over Asset Mesh S=10..50\n";
	printLine();
	
	// Create a vector of asset prices from 10 to 50 with a mesh size of 1 using the MeshArray function
	const std::vector<double> assetArray = MeshArray(10.0, 50.0, 1.0);
	// Create an OptionInput instance with parameters for the mesh pricing, using the same parameters for all asset prices except S which will vary across the mesh
	const OptionInput meshInput {"Mesh Input", 0.25, 65.0, 0.30, 0.08, 10.0, 0.08};
	// Calculate the call and put option prices across the asset price mesh using the europeanPriceAcrossS function, which creates a EuropeanOption for each asset price and calculates the price
	const auto meshCallPrices = europeanPriceAcrossS(meshInput, OptionType::Call, assetArray);
	const auto meshPutPrices = europeanPriceAcrossS(meshInput, OptionType::Put, assetArray);
	// Print the calculated call and put prices across the asset price mesh using the printSeries function
	printSeries(assetArray, meshCallPrices, "Call prices on S mesh : ");
	printSeries(assetArray, meshPutPrices, "Put prices on S mesh :");

	// A.d) Extend part c and compute option prices as a function of i) expiry time, ii) volatility, or iii) any of the option pricing parameters.
	printLine();
	std::cout << "A.d) Matrix Pricer (parameter matrix -> price matrix)\n";
	printLine();
	// Define the base option input and sweep values for time to maturity, volatility, and risk-free rate
	const OptionInput europeanBase {"Base", 0.25, 65.0, 0.30, 0.08, 60.0, 0.08};
	// Create sweep values for time to maturity (T), volatility (sigma), and risk-free rate (r) using the MeshArray function
	const std::vector<double> tSweep = MeshArray(0.25, 2.25, 0.50);
	const std::vector<double> sigmaSweep = MeshArray(0.10, 0.50, 0.10);
	const std::vector<double> rSweep = MeshArray(0.00, 0.12, 0.03);

	// Build input matrices: each sweep value becomes its own row (N×1 layout: one input per row)
	const InputMatrix tInputMatrix = buildInputMatrixFromSweep(europeanBase, tSweep, OptionParameter::T, "T");
	const InputMatrix sigmaInputMatrix = buildInputMatrixFromSweep(europeanBase, sigmaSweep, OptionParameter::Sigma, "sig");
	const InputMatrix rInputMatrix = buildInputMatrixFromSweep(europeanBase, rSweep, OptionParameter::R, "r");

	// Evaluate the price for each input in the matrices using the evaluateMatrix function, which applies a pricing functor to each OptionInput in the matrix and returns a matrix of calculated prices
	auto callPriceByT = evaluateMatrix(tInputMatrix, [](const OptionInput& input)
	{
		EuropeanOption option(OptionType::Call, input.T, input.K, input.sigma, input.r, input.S, input.b);
		return option.price();
	});
	auto callPriceBySigma = evaluateMatrix(sigmaInputMatrix, [](const OptionInput& input)
	{
		EuropeanOption option(OptionType::Call, input.T, input.K, input.sigma, input.r, input.S, input.b);
		return option.price();
	});
	auto callPriceByR = evaluateMatrix(rInputMatrix, [](const OptionInput& input)
	{
		EuropeanOption option(OptionType::Call, input.T, input.K, input.sigma, input.r, input.S, input.b);
		return option.price();
	});

	// Print the calculated price matrices for each parameter sweep using the printMatrix function
	printMatrix(callPriceByT, "European call price matrix (T sweep):");
	printMatrix(callPriceBySigma, "European call price matrix (sigma sweep):");
	printMatrix(callPriceByR, "European call price matrix (r sweep):");

	// A: Option Sensitivities, aka the Greeks

	printLine();
	std::cout << "A: Option Sensitivities, aka the Greeks\n";

	// a) Implement the above formulae for gamma for call and put future option pricing using the data set: K = 100, S = 105, T = 0.5, r = 0.1, b = 0 and sig = 0.36.

	printLine();
	std::cout << "Greeks.a) Compute gamma for the given futures option:\n";
	printLine();

	// Create an OptionInput instance with the specified parameters for the futures option, and create EuropeanOption instances for both call and put options to calculate the Greeks
	const OptionInput futuresGreeks {"Futures Greeks", 1.45, 120.0, 0.51, 0.045, 108.0, 0.0}; //T, 
	EuropeanOption callGreeks(OptionType::Call, futuresGreeks.T, futuresGreeks.K, futuresGreeks.sigma, futuresGreeks.r, futuresGreeks.S, futuresGreeks.b);
	EuropeanOption putGreeks(OptionType::Put, futuresGreeks.T, futuresGreeks.K, futuresGreeks.sigma, futuresGreeks.r, futuresGreeks.S, futuresGreeks.b);

	// Calculate and print the exact delta and gamma for both call and put options using the delta() and gamma() methods of the EuropeanOption class, along with the target values for verification
	std::cout << "Exact call delta=" << callGreeks.delta() << " (target 0.5946)\n";
	std::cout << "Exact put delta =" << putGreeks.delta() << " (target -0.3566)\n";
	std::cout << "Exact gamma     =" << callGreeks.gamma() << " (same for call/put)\n";

	// b) Compute call delta price for a monotonically increasing range of underlying values of S, for example 10, 11, 12, …, 50.

	printLine();
	std::cout << "Greeks.b) Compute call delta across asset price mesh S=10..50\n";
	printLine();

	// Calculate the call delta across the asset price mesh using the europeanDeltaAcrossS function using assetArray created earlier
	const auto meshCallDelta = europeanDeltaAcrossS(futuresGreeks, OptionType::Call, assetArray);
	// Print the calculated call delta across the asset price mesh using the printSeries function
	printSeries(assetArray, meshCallDelta, "Call delta on S mesh:");

	// c) Incorporate this into your above matrix pricer code, so you can input a matrix of option parameters and receive a matrix of either Delta or Gamma as the result.

	printLine();
	std::cout << "Greeks.c) Matrix pricer for Delta and Gamma\n";
	printLine();

	//Reuse the sigmaInputMatrix created earlier to evaluate the delta and gamma across the sigma sweep
	// Evaluate the delta and gamma for each input in the sigma sweep matrix using the evaluateMatrix function
	auto greekMatrixDelta = evaluateMatrix(sigmaInputMatrix, [](const OptionInput& input)
	{
		EuropeanOption option(OptionType::Call, input.T, input.K, input.sigma, input.r, input.S, input.b);
		return option.delta();
	});
	auto greekMatrixGamma = evaluateMatrix(sigmaInputMatrix, [](const OptionInput& input)
	{
		EuropeanOption option(OptionType::Call, input.T, input.K, input.sigma, input.r, input.S, input.b);
		return option.gamma();
	});

	// Print the calculated delta and gamma matrices for the sigma sweep using the printMatrix function
	printMatrix(greekMatrixDelta, "Delta matrix:");
	printMatrix(greekMatrixGamma, "Gamma matrix:");

	// d) Perform the same calculations as in parts a and b, but now using divided differences. Compare the accuracy with various values of the parameter h

	printLine();
	std::cout << "Greeks.d) Finite difference approximations for call delta and gamma\n";
	printLine();
	
	// Set the accurate call delta and gamma values calculated earlier as the target for comparison with the finite difference approximations
	const double exactCallDelta = callGreeks.delta();
	const double exactCallGamma = callGreeks.gamma();

	// Use MeshArray to create a range of h values from large to small so the error convergence is visible.
	const std::vector<double> hMesh = MeshArray(0.1, 50.0, 5.0); // h values from 0.1 to 50 with a step of 5.0

	// Iterate through each h value in the hMesh
	for (double h : hMesh)
	{
		// Calculate the finite difference approximations for delta and gamma using the finiteDifferenceDelta and finiteDifferenceGamma functions
		const double fdDelta = finiteDifferenceDelta(futuresGreeks, OptionType::Call, h);
		const double fdGamma = finiteDifferenceGamma(futuresGreeks, OptionType::Call, h);
		// Calculate the absolute errors for delta and gamma by comparing the finite difference approximations to the exact values
		const double deltaAbsError = std::fabs(fdDelta - exactCallDelta);
		const double gammaAbsError = std::fabs(fdGamma - exactCallGamma);

		// Print the h value along with the finite difference approximations for delta and gamma, and their absolute errors compared to the exact values
		std::cout << "h=" << h
			<< " | FD call delta = " << fdDelta
			<< " | |delta error| = " << deltaAbsError
			<< " | FD call gamma = " << fdGamma
			<< " | |gamma error| = " << gammaAbsError
			<< "\n";
	}

	// B: Perpetual American Options

	printLine();
	std::cout << "B) Perpetual American Options\n";
	printLine();

	//B.b) Test the data with K = 100, sig = 0.1, r = 0.1, b = 0.02, S = 110 (check C = 18.5035, P = 3.03106)

	std::cout << "B.b) Perpetual American option pricing for K=100, sig=0.1, r=0.1, b=0.02, S=110\n";
	printLine();

	// Create the test data for perpetual American options using struct OptionInput
	const OptionInput perpetualCase {"Perpetual Test", 0.0, 100.0, 0.1, 0.1, 110.0, 0.02}; // Set T as 0 for placeholder since it's not used in perpetual option pricing
	// Calculate the perpetual American call and put prices using the perpetualPrice function
	const double perpCall = perpetualPrice(perpetualCase, OptionType::Call);
	const double perpPut = perpetualPrice(perpetualCase, OptionType::Put);
	
	// Print the calculated perpetual American call and put prices along with the target values for verification
	std::cout << "Perpetual test | C=" << perpCall << " (target 18.5035)"
		<< " | P=" << perpPut << " (target 3.03106)\n";

	//B.c) Compute call and put option price for a monotonically increasing range of underlying values of S, for example 10, 11, 12, …, 50.

	printLine();
	std::cout << "B.c) Compute perpetual American call and put prices across asset price mesh S=10..50\n";
	printLine();

	// Reuse assetArray created earlier to calculate perpetual American option prices across the asset price mesh
	// Calculate the perpetual American call and put prices across the asset price mesh using the perpetualPriceAcrossS function
	const auto perpCallMesh = perpetualPriceAcrossS(perpetualCase, OptionType::Call, assetArray);
	const auto perpPutMesh = perpetualPriceAcrossS(perpetualCase, OptionType::Put, assetArray);
	// Print the calculated perpetual American call and put prices across the asset price mesh using the printSeries function
	printSeries(assetArray, perpCallMesh, "Perpetual call prices on S mesh:");
	printSeries(assetArray, perpPutMesh, "Perpetual put prices on S mesh:");

	// B.d) Incorporate this into your above matrix pricer code, so you can input a matrix of option parameters and receive a matrix of Perpetual American option prices.

	printLine();
	std::cout << "B.d) Matrix pricer for perpetual American options\n";
	printLine();
	
	// Create sweep values for risk-free rate (r) using the MeshArray function to generate a range of r values for the perpetual American option pricing
	const std::vector<double> perpetualRSweep = MeshArray(0.05, 0.25, 0.05);
	// Build an input matrix for the perpetual American option pricing: each r value becomes its own row
	const InputMatrix perpetualMatrix = buildInputMatrixFromSweep(perpetualCase, perpetualRSweep, OptionParameter::R, "r");

	// Evaluate the perpetual American call and put prices for each input in the matrix using the evaluateMatrix function, which applies the perpetualPrice functor to each OptionInput in the matrix
	auto perpetualCallMatrix = evaluateMatrix(perpetualMatrix, [](const OptionInput& input)
	{
		return perpetualPrice(input, OptionType::Call);
	});
	auto perpetualPutMatrix = evaluateMatrix(perpetualMatrix, [](const OptionInput& input)
	{
		return perpetualPrice(input, OptionType::Put);
	});

	// Print the calculated perpetual American call and put price matrices for the risk-free rate sweep using the printMatrix function
	printMatrix(perpetualCallMatrix, "Perpetual American call matrix (r sweep):");
	printMatrix(perpetualPutMatrix, "Perpetual American put matrix (r sweep):");

	return 0;
}

