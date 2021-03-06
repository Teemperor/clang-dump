// RUN: %clang_cc1 -verify -fopenmp -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -verify -fopenmp -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o - | FileCheck %s
// RUN: %clang_cc1 -verify -fopenmp -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -emit-pch -o %t
// RUN: %clang_cc1 -verify -fopenmp -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -o - | FileCheck %s

// RUN: %clang_cc1 -verify -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -verify -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o -| FileCheck %s --check-prefix SIMD-ONLY
// RUN: %clang_cc1 -verify -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -emit-pch -o %t
// RUN: %clang_cc1 -verify -fopenmp-simd -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -verify -o - | FileCheck %s --check-prefix SIMD-ONLY

// expected-no-diagnostics

// SIMD-ONLY-NOT: {{__kmpc|__tgt}}

// CHECK-NOT: define {{.*}}{{baz1|baz4|maini1}}
// CHECK-NOT: @{{hhh|ggg|fff|eee}} =
// CHECK-DAG: @aaa = external global i32,
// CHECK-DAG: @bbb = global i32 0,
// CHECK-DAG: @ccc = external global i32,
// CHECK-DAG: @ddd = global i32 0,
// CHECK-DAG: @hhh_decl_tgt_link_ptr = common global i32* null
// CHECK-DAG: @ggg_decl_tgt_link_ptr = common global i32* null
// CHECK-DAG: @fff_decl_tgt_link_ptr = common global i32* null
// CHECK-DAG: @eee_decl_tgt_link_ptr = common global i32* null
// CHECK-DAG: @{{.*}}maini1{{.*}}aaa = internal global i64 23,
// CHECK-DAG: @b = global i32 15,
// CHECK-DAG: @d = global i32 0,
// CHECK-DAG: @c = external global i32,
// CHECK-DAG: @globals = global %struct.S zeroinitializer,
// CHECK-DAG: [[STAT:@.+stat]] = internal global %struct.S zeroinitializer,
// CHECK-DAG: [[STAT_REF:@.+]] = internal constant %struct.S* [[STAT]]
// CHECK-DAG: @out_decl_target = global i32 0,
// CHECK-DAG: @llvm.used = appending global [6 x i8*] [i8* bitcast (void ()* @__omp_offloading__{{.+}}_globals_l[[@LINE+69]]_ctor to i8*), i8* bitcast (void ()* @__omp_offloading__{{.+}}_stat_l[[@LINE+70]]_ctor to i8*),
// CHECK-DAG: @llvm.compiler.used = appending global [1 x i8*] [i8* bitcast (%struct.S** [[STAT_REF]] to i8*)],

// CHECK-DAG: define {{.*}}i32 @{{.*}}{{foo|bar|baz2|baz3|FA|f_method}}{{.*}}()
// CHECK-DAG: define {{.*}}void @{{.*}}TemplateClass{{.*}}(%class.TemplateClass* %{{.*}})
// CHECK-DAG: define {{.*}}i32 @{{.*}}TemplateClass{{.*}}f_method{{.*}}(%class.TemplateClass* %{{.*}})
// CHECK-DAG: define {{.*}}void @__omp_offloading__{{.*}}_globals_l[[@LINE+63]]_ctor()

#ifndef HEADER
#define HEADER

#pragma omp declare target
extern int aaa;
int bbb = 0;
extern int ccc;
int ddd = 0;
#pragma omp end declare target

extern int eee;
int fff = 0;
extern int ggg;
int hhh = 0;
#pragma omp declare target link(eee, fff, ggg, hhh)

int out_decl_target = 0;
#pragma omp declare target
void lambda () {
#ifdef __cpp_lambdas
  (void)[&] { (void)out_decl_target; };
#else
#pragma clang __debug captured
  {
    (void)out_decl_target;
  }
#endif
};
#pragma omp end declare target

template <typename T>
class TemplateClass {
  T a;
public:
  TemplateClass() {}
  T f_method() const { return a; }
};

int foo();

int baz1();

int baz2();

int baz4() { return 5; }

template <typename T>
T FA() {
  TemplateClass<T> s;
  return s.f_method();
}

#pragma omp declare target
struct S {
  int a;
  S(int a) : a(a) {}
};

int foo() { return 0; }
int b = 15;
int d;
S globals(d);
static S stat(d);
#pragma omp end declare target
int c;

int bar() { return 1 + foo() + bar() + baz1() + baz2(); }

int maini1() {
  int a;
  static long aa = 32 + bbb + ccc + fff + ggg;
// CHECK-DAG: define weak void @__omp_offloading_{{.*}}maini1{{.*}}_l[[@LINE+1]](i32* dereferenceable{{.*}}, i64 {{.*}}, i64 {{.*}})
#pragma omp target map(tofrom \
                       : a, b)
  {
    S s(a);
    static long aaa = 23;
    a = foo() + bar() + b + c + d + aa + aaa + FA<int>();
  }
  return baz4();
}

int baz3() { return 2 + baz2(); }
int baz2() {
// CHECK-DAG: define weak void @__omp_offloading_{{.*}}baz2{{.*}}_l[[@LINE+1]](i64 {{.*}})
#pragma omp target parallel
  ++c;
  return 2 + baz3();
}

extern int create() throw();

static __typeof(create) __t_create __attribute__((__weakref__("__create")));

int baz5() {
  bool a;
// CHECK-DAG: define weak void @__omp_offloading_{{.*}}baz5{{.*}}_l[[@LINE+1]](i64 {{.*}})
#pragma omp target
  a = __extension__(void *) & __t_create != 0;
  return a;
}

// CHECK-DAG: declare extern_weak signext i32 @__create()

// CHECK-NOT: define {{.*}}{{baz1|baz4|maini1}}

// CHECK-DAG: !{i32 1, !"aaa", i32 0, i32 {{[0-9]+}}}
// CHECK-DAG: !{i32 1, !"ccc", i32 0, i32 {{[0-9]+}}}

#endif // HEADER
