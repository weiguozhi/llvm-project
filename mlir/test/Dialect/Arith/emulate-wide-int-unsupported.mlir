// RUN: mlir-opt --arith-emulate-wide-int="widest-int-supported=32" \
// RUN:   --split-input-file --verify-diagnostics %s

// Make sure we do not crash on unsupported types.

// Unsupported result type in `arith.extsi`.
func.func @unsupported_result_integer(%arg0: i64) -> i64 {
  // expected-error@+1 {{failed to legalize operation 'arith.extsi' that was explicitly marked illegal}}
  %0 = arith.extsi %arg0: i64 to i128
  %2 = arith.muli %0, %0 : i128
  %3 = arith.trunci %2 : i128 to i64
  return %3 : i64
}

// -----

// Unsupported result type in `arith.extsi`.
func.func @unsupported_result_vector(%arg0: vector<4xi64>) -> vector<4xi64> {
  // expected-error@+1 {{failed to legalize operation 'arith.extsi' that was explicitly marked illegal}}
  %0 = arith.extsi %arg0: vector<4xi64> to vector<4xi128>
  %2 = arith.muli %0, %0 : vector<4xi128>
  %3 = arith.trunci %2 : vector<4xi128> to vector<4xi64>
  return %3 : vector<4xi64>
}

// -----

// Unsupported function return type.
// expected-error@+1 {{failed to legalize operation 'func.func' that was explicitly marked illegal}}
func.func @unsupported_return_type(%arg0: vector<4xi64>) -> vector<4xi128> {
  %0 = arith.extsi %arg0: vector<4xi64> to vector<4xi128>
  return %0 : vector<4xi128>
}

// -----

// Unsupported function argument type.
// expected-error@+1 {{failed to legalize operation 'func.func' that was explicitly marked illegal}}
func.func @unsupported_argument_type(%arg0: vector<4xi128>) -> vector<4xi64> {
  %0 = arith.trunci %arg0: vector<4xi128> to vector<4xi64>
  return %0 : vector<4xi64>
}

// -----

// Ensure this case not crash
func.func @unsupported_vector(%arg0: vector<2xi1>) {
  // expected-error@+1 {{failed to legalize unresolved materialization from ('vector<2x2xi32>') to ('vector<2xi64>') that remained live after conversion}}
  %cst_0 = arith.constant dense<0> : vector<2xi64>
  // expected-note@+1 {{see existing live user here}}
  %0 = vector.mask %arg0 { vector.multi_reduction <xor>, %cst_0, %cst_0 [] : vector<2xi64> to vector<2xi64> } : vector<2xi1> -> vector<2xi64>
  return
}
