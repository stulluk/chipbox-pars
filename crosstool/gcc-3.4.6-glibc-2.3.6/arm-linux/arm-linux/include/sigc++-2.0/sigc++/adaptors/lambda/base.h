// -*- c++ -*-
/* Do not edit! -- generated file */
#ifndef _SIGC_LAMBDA_BASE_HPP_
#define _SIGC_LAMBDA_BASE_HPP_
#include <sigc++/adaptors/adaptor_trait.h>

namespace sigc {

/** @defgroup lambdas Lambdas
 * libsigc++ ships with basic lambda functionality and the sigc::group adaptor that uses lambdas to transform a functor's parameter list.
 *
 * The lambda selectors sigc::_1, sigc::_2, ..., sigc::_9 are used to select the
 * first, second, ..., nineth argument from a list.
 *
 * @par Examples:
 *   @code
 *   std::cout << sigc::_1(10,20,30); // returns 10
 *   std::cout << sigc::_2(10,20,30); // returns 20
 *   ...
 *   @endcode
 *
 * Operators are defined so that lambda selectors can be used e.g. as placeholders in
 * arithmetic expressions.
 *
 * @par Examples:
 *   @code
 *   std::cout << (sigc::_1 + 5)(3); // returns (3 + 5)
 *   std::cout << (sigc::_1 * sigc::_2)(7,10); // returns (7 * 10)
 */

// dummy structure to keep track of the where we are.
struct lambda_base : public adaptor_base {};

template <class T_type> struct lambda;


namespace internal {

template <class T_type, bool I_islambda = is_base_and_derived<lambda_base, T_type>::value> struct lambda_core;

template <class T_type>
struct lambda_core<T_type, true> : public lambda_base
{
  template <class T_arg1=void,class T_arg2=void,class T_arg3=void,class T_arg4=void,class T_arg5=void,class T_arg6=void,class T_arg7=void>
  struct deduce_result_type
    { typedef typename T_type::template deduce_result_type<typename type_trait<T_arg1>::pass,typename type_trait<T_arg2>::pass,typename type_trait<T_arg3>::pass,typename type_trait<T_arg4>::pass,typename type_trait<T_arg5>::pass,typename type_trait<T_arg6>::pass,typename type_trait<T_arg7>::pass>::type type; };
  typedef typename T_type::result_type result_type;
  typedef T_type lambda_type;

  result_type
  operator()() const;

  template <class T_arg1>
  typename deduce_result_type<T_arg1>::type
  operator ()(T_arg1 _A_1) const 
    { return value_.LIBSIGC_TEMPLATE_PREFIX SIGC_WORKAROUND_OPERATOR_PARENTHESES<typename type_trait<T_arg1>::pass>
             (_A_1); 
    }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1>
  typename deduce_result_type<T_arg1>::type
  sun_forte_workaround(T_arg1 _A_1) const
    { return operator()(_A_1); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2>
  typename deduce_result_type<T_arg1,T_arg2>::type
  operator ()(T_arg1 _A_1,T_arg2 _A_2) const 
    { return value_.LIBSIGC_TEMPLATE_PREFIX SIGC_WORKAROUND_OPERATOR_PARENTHESES<typename type_trait<T_arg1>::pass,typename type_trait<T_arg2>::pass>
             (_A_1,_A_2); 
    }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2>
  typename deduce_result_type<T_arg1,T_arg2>::type
  sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2) const
    { return operator()(_A_1,_A_2); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3>::type
  operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3) const 
    { return value_.LIBSIGC_TEMPLATE_PREFIX SIGC_WORKAROUND_OPERATOR_PARENTHESES<typename type_trait<T_arg1>::pass,typename type_trait<T_arg2>::pass,typename type_trait<T_arg3>::pass>
             (_A_1,_A_2,_A_3); 
    }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3>::type
  sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3) const
    { return operator()(_A_1,_A_2,_A_3); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4>::type
  operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4) const 
    { return value_.LIBSIGC_TEMPLATE_PREFIX SIGC_WORKAROUND_OPERATOR_PARENTHESES<typename type_trait<T_arg1>::pass,typename type_trait<T_arg2>::pass,typename type_trait<T_arg3>::pass,typename type_trait<T_arg4>::pass>
             (_A_1,_A_2,_A_3,_A_4); 
    }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4>::type
  sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4) const
    { return operator()(_A_1,_A_2,_A_3,_A_4); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4,T_arg5>::type
  operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5) const 
    { return value_.LIBSIGC_TEMPLATE_PREFIX SIGC_WORKAROUND_OPERATOR_PARENTHESES<typename type_trait<T_arg1>::pass,typename type_trait<T_arg2>::pass,typename type_trait<T_arg3>::pass,typename type_trait<T_arg4>::pass,typename type_trait<T_arg5>::pass>
             (_A_1,_A_2,_A_3,_A_4,_A_5); 
    }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4,T_arg5>::type
  sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5) const
    { return operator()(_A_1,_A_2,_A_3,_A_4,_A_5); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4,T_arg5,T_arg6>::type
  operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6) const 
    { return value_.LIBSIGC_TEMPLATE_PREFIX SIGC_WORKAROUND_OPERATOR_PARENTHESES<typename type_trait<T_arg1>::pass,typename type_trait<T_arg2>::pass,typename type_trait<T_arg3>::pass,typename type_trait<T_arg4>::pass,typename type_trait<T_arg5>::pass,typename type_trait<T_arg6>::pass>
             (_A_1,_A_2,_A_3,_A_4,_A_5,_A_6); 
    }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4,T_arg5,T_arg6>::type
  sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6) const
    { return operator()(_A_1,_A_2,_A_3,_A_4,_A_5,_A_6); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6,class T_arg7>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4,T_arg5,T_arg6,T_arg7>::type
  operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6,T_arg7 _A_7) const 
    { return value_.LIBSIGC_TEMPLATE_PREFIX SIGC_WORKAROUND_OPERATOR_PARENTHESES<typename type_trait<T_arg1>::pass,typename type_trait<T_arg2>::pass,typename type_trait<T_arg3>::pass,typename type_trait<T_arg4>::pass,typename type_trait<T_arg5>::pass,typename type_trait<T_arg6>::pass,typename type_trait<T_arg7>::pass>
             (_A_1,_A_2,_A_3,_A_4,_A_5,_A_6,_A_7); 
    }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6,class T_arg7>
  typename deduce_result_type<T_arg1,T_arg2,T_arg3,T_arg4,T_arg5,T_arg6,T_arg7>::type
  sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6,T_arg7 _A_7) const
    { return operator()(_A_1,_A_2,_A_3,_A_4,_A_5,_A_6,_A_7); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  lambda_core() {}

  explicit lambda_core(const T_type& v)
    : value_(v) {}

  T_type value_;
};

template <class T_type>
typename lambda_core<T_type, true>::result_type
lambda_core<T_type, true>::operator()() const
  { return value_(); }


template <class T_type>
struct lambda_core<T_type, false> : public lambda_base
{
  template <class T_arg1=void,class T_arg2=void,class T_arg3=void,class T_arg4=void,class T_arg5=void,class T_arg6=void,class T_arg7=void>
  struct deduce_result_type
    { typedef T_type type; };
  typedef T_type result_type; // all operator() overloads return T_type.
  typedef lambda<T_type> lambda_type;

  result_type operator()() const;

  template <class T_arg1>
  result_type operator ()(T_arg1 _A_1) const 
    { return value_; }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1>
  result_type sun_forte_workaround(T_arg1 _A_1) const
    { return operator()(_A_1); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2>
  result_type operator ()(T_arg1 _A_1,T_arg2 _A_2) const 
    { return value_; }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2>
  result_type sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2) const
    { return operator()(_A_1,_A_2); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3>
  result_type operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3) const 
    { return value_; }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3>
  result_type sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3) const
    { return operator()(_A_1,_A_2,_A_3); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4>
  result_type operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4) const 
    { return value_; }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4>
  result_type sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4) const
    { return operator()(_A_1,_A_2,_A_3,_A_4); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5>
  result_type operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5) const 
    { return value_; }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5>
  result_type sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5) const
    { return operator()(_A_1,_A_2,_A_3,_A_4,_A_5); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6>
  result_type operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6) const 
    { return value_; }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6>
  result_type sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6) const
    { return operator()(_A_1,_A_2,_A_3,_A_4,_A_5,_A_6); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6,class T_arg7>
  result_type operator ()(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6,T_arg7 _A_7) const 
    { return value_; }

  #ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  template <class T_arg1,class T_arg2,class T_arg3,class T_arg4,class T_arg5,class T_arg6,class T_arg7>
  result_type sun_forte_workaround(T_arg1 _A_1,T_arg2 _A_2,T_arg3 _A_3,T_arg4 _A_4,T_arg5 _A_5,T_arg6 _A_6,T_arg7 _A_7) const
    { return operator()(_A_1,_A_2,_A_3,_A_4,_A_5,_A_6,_A_7); }
  #endif //SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD

  explicit lambda_core(typename type_trait<T_type>::take v)
    : value_(v) {}

  T_type value_;
};

template <class T_type>
typename lambda_core<T_type, false>::result_type lambda_core<T_type, false>::operator()() const
  { return value_; }

} /* namespace internal */


template <class T_action, class T_functor, bool I_islambda>
void visit_each(const T_action& _A_action,
                const internal::lambda_core<T_functor, I_islambda>& _A_target)
{
  visit_each(_A_action, _A_target.value_);
}


// forward declarations for lambda operators other<subscript> and other<assign>
template <class T_type>
struct other;
struct subscript;
struct assign;

template <class T_action, class T_type1, class T_type2>
struct lambda_operator;


// lambda
template <class T_type>
struct lambda : public internal::lambda_core<T_type>
{
  typedef lambda<T_type> self;

  lambda()
    {}

  lambda(typename type_trait<T_type>::take v)
    : internal::lambda_core<T_type>(v) 
    {}

  // operators for other<subscript>
  template <class T_arg>
  lambda<lambda_operator<other<subscript>, self, T_arg> >
  operator [] (const lambda<T_arg>& a) const
    { return lambda<lambda_operator<other<subscript>, self, T_arg> >(lambda_operator<other<subscript>, self, T_arg>(this->value_, a.value_)); }
  template <class T_arg>
  lambda<lambda_operator<other<subscript>, self, T_arg> >
  operator [] (const T_arg& a) const
    { return lambda<lambda_operator<other<subscript>, self, T_arg> >(lambda_operator<other<subscript>, self, T_arg>(this->value_, a)); }

  // operators for other<assign>
  template <class T_arg>
  lambda<lambda_operator<other<assign>, self, T_arg> >
  operator = (const lambda<T_arg>& a) const
    { return lambda<lambda_operator<other<assign>, self, T_arg> >(lambda_operator<other<assign>, self, T_arg>(this->value_, a.value_)); }
  template <class T_arg>
  lambda<lambda_operator<other<assign>, self, T_arg> >
  operator = (const T_arg& a) const
    { return lambda<lambda_operator<other<assign>, self, T_arg> >(lambda_operator<other<assign>, self, T_arg>(this->value_, a)); }
};


template <class T_action, class T_type>
void visit_each(const T_action& _A_action,
                const lambda<T_type>& _A_target)
{
  visit_each(_A_action, _A_target.value_);
}


template <class T_type>
lambda<T_type> constant(const T_type& v)
{ return lambda<T_type>(v); }

template <class T_type>
lambda<T_type&> var(T_type& v)
{ return lambda<T_type&>(v); }

template <class T_type>
lambda<const T_type&> var(const T_type& v)
{ return lambda<const T_type&>(v); }

} /* namespace sigc */

#endif /* _SIGC_LAMBDA_BASE_HPP_ */
