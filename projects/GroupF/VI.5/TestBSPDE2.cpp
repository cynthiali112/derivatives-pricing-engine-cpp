// TestBSPDE2.cpp
//
// Testing 1 factor BS model.
//
// (C) Datasim Education BV 2005-2011
//

// Fix for std::byte vs Windows COM byte ambiguity in C++17
#define _HAS_STD_BYTE 0

#include "FdmDirector.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>
using namespace std;

#ifdef ENABLE_EXCEL_OUTPUT
#include "UtilitiesDJD/ExcelDriver/ExcelDriverLite.hpp"
#endif
#include "UtilitiesDJD/Matrices/NestedMatrix.hpp"

namespace BS // Black Scholes
{
	double sig = 0.3;
	double K = 65.0;
	double T = 0.25;
	double r = 0.08;
	double D = 0.0; // aka q

	double mySigma (double x, double t)
	{

		double sigmaS = sig*sig;

		return 0.5 * sigmaS * x * x;
	}

	double myMu (double x, double t)
	{
		
		return (r - D) * x;
	
	}

	double myB (double x, double t)
	{	
	
		return  -r;
	}

	double myF (double x, double t)
	{
		return 0.0;
	}

	double myBCL (double t)		
	{
		// Put
		return K *exp(-r * t);
	}

	double myBCR (double t)
	{
			
		// Put
		return 0.0; // P
	}

	double myIC (double x)
	{ // Payoff 
	
		// Put
		return max(K - x, 0.0);
	}

}

// Create struct Batch to hold parameters for each test case.
struct Batch
{
	double T;
	double K;
	double sig;
	double r;
	double S;
	double exactCall;
	double exactPut;
};

// Create struct BatchResult to hold results for each test case.
struct BatchResult
{
	double fdmCall;
	double fdmPut;
	double givenCall;
	double givenPut;
	double givenCallError;
	double givenPutError;
};

// Function to calculate a stable number of time steps N based on the stability condition for explicit FDM.
long stableN(double T, double sig, double Smax, double h)
{
	// Stability condition for explicit FDM: N >= (T * sig^2 * Smax^2) / h^2
	const double minN = (T * sig * sig * Smax * Smax) / (h * h);
	// Return the maximum of 10,000 and the calculated minimum N, rounded up to the nearest whole number.
	return (std::max)(10000L - 1L, static_cast<long>(std::ceil(minN)) + 1L);
}

// Function to find the index of the mesh point corresponding to a given asset price S in the mesh defined by x.
long meshIndex(double s, const std::vector<double>& x)
{
	// Ensure the mesh has at least two points to avoid division by zero when calculating h.
	if (x.size() < 2)
	{
		// If the mesh does not contain at least two points, throw a runtime error.
		throw std::runtime_error("Mesh must contain at least two points.");
	}

	// Calculate the mesh spacing h using the first two points in the mesh.
	const double h = x[1] - x[0];
	// Calculate the index of the mesh point corresponding to the asset price s using the formula: index = (s - x[0]) / h, rounded to the nearest whole number.
	const long index = static_cast<long>(std::llround((s - x.front()) / h));

	// Check if the calculated index is within the bounds of the mesh. If not, throw an out_of_range exception.
	if (index < 0 || index >= static_cast<long>(x.size()))
	{
		throw std::out_of_range("Asset lies outside the FDM mesh.");
	}

	// Check if the asset price s is sufficiently close to the mesh point at the calculated index. If not, throw a runtime error indicating that the asset is not a mesh node and suggesting to increase J or reintroduce interpolation.
	if (std::fabs(x[static_cast<std::size_t>(index)] - s) > 1.0e-10)
	{
		throw std::runtime_error("Asset is not a mesh node. Increase J or reintroduce interpolation.");
	}

	// If all checks pass, return the calculated index.
	return index;
}


int main()
{
	using namespace ParabolicIBVP;
	typedef NestedMatrix<double> NumericMatrix;

	// Assignment of functions
	sigma = BS::mySigma;
	mu = BS::myMu;
	b = BS::myB;
	f = BS::myF;
	BCL = BS::myBCL;
	BCR = BS::myBCR;
	IC = BS::myIC;

	// Define test cases with parameters and exact solutions for call and put options.
	const std::vector<Batch> batches = {
		{0.25, 65.0, 0.30, 0.08, 60.0, 2.13337, 5.84628}, // T=0.25, K=65, sig=0.30, r=0.08, S=60, exactCall=2.13337, exactPut=5.84628
		{1.00, 100.0, 0.20, 0.00, 100.0, 7.96557, 7.96557},
		{1.00, 10.0, 0.50, 0.12, 5.0, 0.204058, 4.07326},
		{30.0, 100.0, 0.30, 0.08, 100.0, 92.17570, 1.24750},
		{0.65, 70.0, 0.35, 0.065, 65.0, 6.49719, 3.46457}
	};
	// Create a vector to hold the results for each batch.
	std::vector<BatchResult> results(batches.size());

	// Set output formatting for console output.
	cout << fixed << setprecision(6);

	#ifdef ENABLE_EXCEL_OUTPUT
	// Create an instance of the Excel driver and make Excel visible for output.
	ExcelDriver& excel = ExcelDriver::Instance();
	excel.MakeVisible(true);
	#endif

	// Loop through each batch of test cases, run the FDM solver, and store results.
	for (std::size_t i = 0; i < batches.size(); ++i)
	{
		const Batch& batch = batches[i];
		BatchResult& result = results[i];

		BS::T = batch.T;
		BS::K = batch.K;
		BS::sig = batch.sig;
		BS::r = batch.r;
		BS::D = 0.0;

		const double Smax = 5.0 * BS::K; // Magix: set Smax to 5 times the strike price K 
		const long J = static_cast<long>(BS::K); // Set J to the strike price K, which determines the number of spatial steps in the FDM mesh.
		const double h = Smax / static_cast<double>(J); // Calculate the spatial step size h based on Smax and J.
		const long N = stableN(BS::T, BS::sig, Smax, h); // Calculate the number of time steps N using the stableN function, which ensures stability of the explicit FDM scheme based on the given parameters.

		// Output the start of the batch processing to the console.
		cout << "Batch " << (i + 1) << " start\n";
		FDMDirector fdir(Smax, BS::T, J, N);
		fdir.doit();

		// Calculate the index of the mesh point corresponding to the asset price S in the FDM mesh and retrieve the FDM solution for the put option at that index
		const long sIndex = meshIndex(batch.S, fdir.xarr);
		result.fdmPut = fdir.current()[static_cast<std::size_t>(sIndex)];
		// Calculate the FDM solution for the call option using the put-call parity relationship: Call = Put + S * exp(-D * T) - K * exp(-r * T).
		result.fdmCall = result.fdmPut + batch.S * std::exp(-BS::D * BS::T) - BS::K * std::exp(-BS::r * BS::T);

		// Store the exact solutions for call and put options from the batch and calculate the errors compared to the FDM results.
		result.givenCall = batch.exactCall;
		result.givenPut = batch.exactPut;
		// Calculate the errors for call and put options by comparing the FDM results with the given exact solutions.
		result.givenCallError = result.fdmCall - result.givenCall;
		result.givenPutError = result.fdmPut - result.givenPut;

		// Output the results and errors for the current batch to the console.
		cout << "  Inputs: T=" << BS::T << ", K=" << BS::K << ", sig=" << BS::sig
			 << ", r=" << BS::r << ", S=" << batch.S << "\n";
		cout << "  FDM   : C=" << result.fdmCall << ", P=" << result.fdmPut << "\n";
		cout << "  Given : C=" << batch.exactCall << ", P=" << batch.exactPut << "\n";
		cout << "  Error : dC=" << result.givenCallError
			 << ", dP=" << result.givenPutError << "\n\n";

		// Create a chart in Excel for the current batch using the FDM results and label it with the batch number and parameters.
		#ifdef ENABLE_EXCEL_OUTPUT
		excel.CreateChart(fdir.xarr, fdir.current(), string("Batch ") + to_string(i + 1), string("S"), string("Put"));
		#endif
	}

	// Create a matrix to hold the accuracy results for all batches and define row and column labels for the Excel output.
	NumericMatrix accuracyMatrix(batches.size(), 11);
	std::list<std::string> rowLabels;
	std::list<std::string> columnLabels;
	columnLabels.push_back("T");
	columnLabels.push_back("K");
	columnLabels.push_back("sig");
	columnLabels.push_back("r");
	columnLabels.push_back("S");
	columnLabels.push_back("FDM C");
	columnLabels.push_back("FDM P");
	columnLabels.push_back("Given C");
	columnLabels.push_back("Given P");
	columnLabels.push_back("dC given");
	columnLabels.push_back("dP given");

	// Output the error summary for all batches to the console and populate the accuracy matrix with the results for Excel output.
	cout << "Error summary\n";
	// Loop through each batch and its corresponding results
	for (std::size_t i = 0; i < batches.size(); ++i)
	{
		// Retrieve the current batch and its results from the vectors.
		const Batch& batch = batches[i];
		const BatchResult& result = results[i];
		// Add a row label for the current batch to the list of row labels for Excel output.
		rowLabels.push_back(string("Batch ") + to_string(i + 1));

		accuracyMatrix(i, 0) = batch.T;
		accuracyMatrix(i, 1) = batch.K;
		accuracyMatrix(i, 2) = batch.sig;
		accuracyMatrix(i, 3) = batch.r;
		accuracyMatrix(i, 4) = batch.S;
		accuracyMatrix(i, 5) = result.fdmCall;
		accuracyMatrix(i, 6) = result.fdmPut;
		accuracyMatrix(i, 7) = result.givenCall;
		accuracyMatrix(i, 8) = result.givenPut;
		accuracyMatrix(i, 9) = result.givenCallError;
		accuracyMatrix(i, 10) = result.givenPutError;

		// Output the error summary for the current batch to the console, including the batch number and the errors for call and put options.
		cout << "  Batch " << (i + 1)
			 << ": dC=" << result.givenCallError
			 << ", dP=" << result.givenPutError << "\n";
	}

	// Add the accuracy matrix to Excel with the defined row and column labels, starting at cell A1.
	#ifdef ENABLE_EXCEL_OUTPUT
	excel.AddMatrix<NumericMatrix>(accuracyMatrix, string("Errors"), rowLabels, columnLabels, 1, 1);
	#endif

	return 0;
}
