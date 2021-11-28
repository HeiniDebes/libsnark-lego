/*
 * bench_lego_utils.hpp
 *
 * 		// Utilities for running benchmarks on LegoGroth for arbitrary-size constraint systems
 *      Author: Matteo Campanelli
 */

#include <libsnark/gadgetlib2/integration.hpp>
#include <libsnark/gadgetlib2/adapters.hpp>

#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/examples/run_lego.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/lego.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>

#include <libsnark/common/default_types/r1cs_gg_ppzksnark_pp.hpp>

#include "benchmark.h"


#include <filesystem>

using namespace std;

template<typename ppT>
struct LegoBenchGadget {

	lego_example<ppT> input_rel;
	lego_keypair<ppT> keypair;

	lego_proof<ppT> prf;
	bool successBit;
		
	LegoBenchGadget(size_t size_pub_input, size_t size_comm_input, size_t num_constraints) :
		input_rel(generate_lego_example_with_field_input<ppT>(num_constraints, size_pub_input, size_comm_input)),
		keypair(lego_kg<ppT>(input_rel.ck, input_rel.r1cs()) )
	{


	}

	void bench_prv(size_t nreps, string label)
	{
		auto prv_fn = [&] {
			prf = lego_prv<ppT>(keypair,  input_rel.x, 
				input_rel.cm, input_rel.opn, input_rel.omega);
		};

		// run benchmarks
		libff::print_header("Benchmarking");
		libff::print_header(label.c_str());
		fmt_time(label, TimeDelta::runAndAverage(prv_fn, nreps));
	}

	// this function must be called after bench_prv is called at least once
	void bench_vfy(size_t nreps, string label)
	{
		
		auto vfy_fn = [&] {
			successBit = lego_vfy<ppT>(keypair, input_rel.x, 
			input_rel.cm, prf);
		};

		// run benchmarks
		libff::print_header("Benchmarking");
		libff::print_header(label.c_str());
		fmt_time(label, TimeDelta::runAndAverage(vfy_fn, nreps));

	}
	
};

