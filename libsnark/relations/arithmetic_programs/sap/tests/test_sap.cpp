/**
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

#include <libff/algebra/curves/mnt/mnt6/mnt6_pp.hpp>
#include <libff/algebra/field_utils/field_utils.hpp>
#include <libff/common/profiling.hpp>
#include <libff/common/utils.hpp>

#include <libsnark/reductions/r1cs_to_sap/r1cs_to_sap.hpp>
#include <libsnark/relations/constraint_satisfaction_problems/r1cs/examples/r1cs_examples.hpp>

using namespace libsnark;

template<typename FieldT>
void test_sap(const size_t sap_degree, const size_t num_inputs, const bool binary_input)
{
    /*
      We construct an instance where the SAP degree is <= sap_degree.
      The R1CS-to-SAP reduction produces SAPs with degree
        (2 * num_constraints + 2 * num_inputs + 1).
      So we generate an instance of R1CS where the number of constraints is
        (sap_degree - 1) / 2 - num_inputs.
    */
    libff::enter_block("Call to test_sap");

    const size_t num_constraints = (sap_degree - 1) / 2 - num_inputs;
    assert(num_constraints >= 1);

    libff::print_indent(); printf("* Requested SAP degree: %zu\n", sap_degree);
    libff::print_indent(); printf("* Actual SAP degree: %zu\n", 2 * num_constraints + 2 * num_inputs + 1);
    libff::print_indent(); printf("* Number of inputs: %zu\n", num_inputs);
    libff::print_indent(); printf("* Number of R1CS constraints: %zu\n", num_constraints);
    libff::print_indent(); printf("* Input type: %s\n", binary_input ? "binary" : "field");

    libff::enter_block("Generate constraint system and assignment");
    r1cs_example<FieldT> example;
    if (binary_input)
    {
        example = generate_r1cs_example_with_binary_input<FieldT>(num_constraints, num_inputs);
    }
    else
    {
        example = generate_r1cs_example_with_field_input<FieldT>(num_constraints, num_inputs);
    }
    libff::leave_block("Generate constraint system and assignment");

    libff::enter_block("Check satisfiability of constraint system");
    assert(example.constraint_system.is_satisfied(example.primary_input, example.auxiliary_input));
    libff::leave_block("Check satisfiability of constraint system");

    const FieldT t = FieldT::random_element(),
    d1 = FieldT::random_element(),
    d2 = FieldT::random_element();

    libff::enter_block("Compute SAP instance 1");
    sap_instance<FieldT> sap_inst_1 = r1cs_to_sap_instance_map(example.constraint_system);
    libff::leave_block("Compute SAP instance 1");

    libff::enter_block("Compute SAP instance 2");
    sap_instance_evaluation<FieldT> sap_inst_2 = r1cs_to_sap_instance_map_with_evaluation(example.constraint_system, t);
    libff::leave_block("Compute SAP instance 2");

    libff::enter_block("Compute SAP witness");
    sap_witness<FieldT> sap_wit = r1cs_to_sap_witness_map(example.constraint_system, example.primary_input, example.auxiliary_input, d1, d2);
    libff::leave_block("Compute SAP witness");

    libff::enter_block("Check satisfiability of SAP instance 1");
    assert(sap_inst_1.is_satisfied(sap_wit));
    libff::leave_block("Check satisfiability of SAP instance 1");

    libff::enter_block("Check satisfiability of SAP instance 2");
    assert(sap_inst_2.is_satisfied(sap_wit));
    libff::leave_block("Check satisfiability of SAP instance 2");

    libff::leave_block("Call to test_sap");
}

int main()
{
    libff::start_profiling();

    libff::mnt6_pp::init_public_params();

    const size_t num_inputs = 10;

    /**
     * due to the specifics of our reduction, we can only get SAPs with odd
     * degrees, so we can only test "special" versions of the domains
     */

    const size_t basic_domain_size_special = (1ul<<libff::mnt6_Fr::s) - 1ul;
    const size_t step_domain_size_special = (1ul<<10) + (1ul<<8) - 1ul;
    const size_t extended_domain_size_special = (1ul<<(libff::mnt6_Fr::s+1)) - 1ul;

    libff::enter_block("Test SAP with binary input");

    test_sap<libff::Fr<libff::mnt6_pp> >(basic_domain_size_special, num_inputs, true);
    test_sap<libff::Fr<libff::mnt6_pp> >(step_domain_size_special, num_inputs, true);
    test_sap<libff::Fr<libff::mnt6_pp> >(extended_domain_size_special, num_inputs, true);

    libff::leave_block("Test SAP with binary input");

    libff::enter_block("Test SAP with field input");

    test_sap<libff::Fr<libff::mnt6_pp> >(basic_domain_size_special, num_inputs, false);
    test_sap<libff::Fr<libff::mnt6_pp> >(step_domain_size_special, num_inputs, false);
    test_sap<libff::Fr<libff::mnt6_pp> >(extended_domain_size_special, num_inputs, false);

    libff::leave_block("Test SAP with field input");
}
