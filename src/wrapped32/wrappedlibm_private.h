#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

// Complex numbers are passed in as an address in the stack

GOW(acos, dEd)
GOW(acosf, fEf)
GOM(__acosf_finite, fEf) //%noE
GOM(__acos_finite, dEd)  //%noE
GOW(acosh, dEd)
GOW(acoshf, fEf)
GOM(__acoshf_finite, fEf) //%noE
GOM(__acosh_finite, dEd)  //%noE
#ifdef HAVE_LD80BITS
GOW(acoshl, DED)
#else
GOW2(acoshl, KEK, acosh)
#endif
#ifdef HAVE_LD80BITS
GOW(acosl, DED)
#else
GOW2(acosl, KEK, acos)
#endif
GOW(asin, dEd)
GOW(asinf, fEf)
GOM(__asinf_finite, fEf) //%noE
GOM(__asin_finite, dEd)  //%noE
GOW(asinh, dEd)
GOW(asinhf, fEf)
#ifdef HAVE_LD80BITS
GOW(asinhl, DED)
#else
GOW2(asinhl, KEK, asinh)
#endif
#ifdef HAVE_LD80BITS
GOW(asinl, DED)
#else
GOW2(asinl, KEK, asin)
#endif
GO(atan, dEd)
GOW(atan2, dEdd)
GOW(atan2f, fEff)
GOM(__atan2f_finite, fEff) //%noE
GOM(__atan2_finite, dEdd)  //%noE
// atan2l   // Weak
GOW(atanf, fEf)
GOW(atanh, dEd)
GOW(atanhf, fEf)
// __atanhf_finite
// __atanh_finite
#ifdef HAVE_LD80BITS
GOW(atanhl, DED)
#else
GOW2(atanhl, KEK, atanh)
#endif
// atanl    // Weak
GOW(cabs, dEdd)     // only 1 arg, but is a double complex
GOW(cabsf, fEff)    // only 1 arg, but is a float complex
// cabsl    // Weak
//GOWS(cacos, pEps)   //%% complex
GOWM(cacosf, UEs)   //%noE
//GOWS(cacosh, pEps)  //%% complex
GOWM(cacoshf, UEs)  //%noE
// cacoshl  // Weak
// cacosl   // Weak
//GOW(carg, dEdd)     // 1arg, double complex
GOW(cargf, fEff)    // 1arg, float complex
// cargl    // Weak
//GOWS(casin, pEps)   //%%  complex
GOWM(casinf, UEs)   //%noE
//GOWS(casinh, pEps)  //%%  complex
GOWM(casinhf, UEs)  //%noE
// casinhl  // Weak
// casinl   // Weak
//GOWS(catan, pEps)   //%%   complex
GOWM(catanf, UEs)   //%noE
//GOWS(catanh, pEps)  //%%   complex
GOWM(catanhf, UEs)  //%noE
// catanhl  // Weak
// catanl   // Weak
GOW(cbrt, dEd)
GOW(cbrtf, fEf)
#ifdef HAVE_LD80BITS
GOW(cbrtl, DED)
#else
GOW2(cbrtl, KEK, cbrt)
#endif
//GOWS(ccos, pEps)    //%%   return complex
GOWM(ccosf, UEs)    //%noE return complex
//GOWS(ccosh, pEps)   //%%   return complex
GOWM(ccoshf, UEs)   //%noE return complex
// ccoshl   // Weak
// ccosl    // Weak
GOW(ceil, dEd)
GOW(ceilf, fEf)
// ceill    // Weak
//GOWS(cexp, pEps)     //%%   return complex
GOWM(cexpf, UEs)     //%noE return complex
// cexpl    // Weak
// cimag    // Weak
// cimagf   // Weak
// cimagl   // Weak
//GOS(clog, pEps)   //%% return a double complex, so ret 4
// clog10   // Weak
// __clog10
// clog10f  // Weak
// __clog10f
// clog10l  // Weak
// __clog10l
GOM(clogf, UEs)    //%noE float complex doesn't trigger the ret 4, but returns an u64!
// clogl    // Weak
// conj // Weak
// conjf    // Weak
// conjl    // Weak
GOW(copysign, dEdd)
GOW(copysignf, fEff)
// copysignl    // Weak
GOW(cos, dEd)
GOW(cosf, fEf)
GOW(cosh, dEd)
GOW(coshf, fEf)
GOM(__coshf_finite, fEf) //%noE
GOM(__cosh_finite, dEd)  //%noE
// coshl    // Weak
// cosl // Weak
//GOWS(cpow, pEpsvvvvs) //%% return complex
GOWM(cpowf, UEsvvs)   //%noE
// cpowl    // Weak
//GOS(cproj, pEps)    //%%   double complex
GOM(cprojf, UEs)    //%noE
// cprojl   // Weak
// creal    // Weak
// crealf   // Weak
// creall   // Weak
//GOWS(csin, pEps)    //%%   return complex
GOWM(csinf, UEs)    //%noE return complex
//GOWS(csinh, pEps)   //%%   return complex
GOWM(csinhf, UEs)   //%noE return complex
// csinhl   // Weak
// csinl    // Weak
//GOWS(csqrt, pEps)   //%%
GOWM(csqrtf, UEs)   //%noE
// csqrtl   // Weak
//GOWS(ctan, pEps)    //%%   return complex
GOWM(ctanf, UEs)    //%noE return complex
//GOWS(ctanh, pEps)   //%%   return complex
GOWM(ctanhf, UEs)   //%noE return complex
// ctanhl   // Weak
// ctanl    // Weak
// __cxa_finalize  // Weak
// drem // Weak
// dremf    // Weak
// dreml    // Weak
GOW(erf, dEd)
GOW(erfc, dEd)
GOW(erfcf, fEf)
#ifdef HAVE_LD80BITS
GOW(erfcl, DED)
#else
GOW2(erfcl, KEK, erfc)
#endif
GOW(erff, fEf)
#ifdef HAVE_LD80BITS
GOW(erfl, DED)
#else
GOW2(erfl, KEK, erf)
#endif
GOW(exp, dEd)
GOW(exp10, dEd)
GOW(exp10f, fEf)
// __exp10f_finite
// __exp10_finite
// exp10l   // Weak
GOW(exp2, dEd)
GOW(exp2f, fEf)
GOM(__exp2f_finite, fEf) //%noE
GOM(__exp2_finite, dEd)  //%noE
// exp2l    // Weak
GOW(expf, fEf)
GOM(__expf_finite, fEf) //%noE
GOM(__exp_finite, dEd)  //%noE
// expl // Weak
GOW(expm1, dEd)
GOW(expm1f, fEf)
// expm1l   // Weak
GOW(fabs, dEd)
GOW(fabsf, fEf)
// fabsl    // Weak
// fdim // Weak
// fdimf    // Weak
// fdiml    // Weak
GO(feclearexcept, iEi)
GO(fedisableexcept, iEi)
GO(feenableexcept, iEi)
GO(fegetenv, iEp)
GO(fegetexcept, iEv)
GO(fegetexceptflag, iEpi)
GO(fegetround, iEv)
GO(feholdexcept, iEp)
GO(feraiseexcept, iEi)
GO(fesetenv, iEp)
GO(fesetexceptflag, iEpi)
GO(fesetround, iEi)
GO(fetestexcept, iEi)
GO(feupdateenv, iEp)
GOW(finite, iEd)
// __finite
GOW(finitef, iEf)
GO(__finitef, iEf)
// finitel  // Weak
// __finitel
GOW(floor, dEd)
GOW(floorf, fEf)
// floorl   // Weak
GOW(fma, dEddd)
GOW(fmaf, fEfff)
// fmal // Weak
GOW(fmax, dEdd)
GOW(fmaxf, fEff)
// fmaxl    // Weak
GOW(fmin, dEdd)
GOW(fminf, fEff)
// fminl    // Weak
GOW(fmod, dEdd)
GOW(fmodf, fEff)
// __fmodf_finite
// __fmod_finite
#ifdef HAVE_LD80BITS
GOW(fmodl, DEDD)
#else
GOW2(fmodl, KEKK, fmod)
#endif
GO(__fpclassify, iEd)
GO(__fpclassifyf, iEf)
GOW(frexp, dEdp)
GOW(frexpf, fEfp)
#ifdef HAVE_LD80BITS
GOW(frexpl, DEDp)
#else
GOW2(frexpl, KEKp, frexp)
#endif
// gamma    // Weak
// gammaf   // Weak
// __gammaf_r_finite
// gammal   // Weak
// __gamma_r_finite
// __gmon_start__  // Weak
GOW(hypot, dEdd)
GOW(hypotf, fEff)
GOM(__hypotf_finite, fEff) //%noE
GOM(__hypot_finite, dEdd)  //%noE
// hypotl   // Weak
GOW(ilogb, iEd)
GOW(ilogbf, iEf)
// ilogbl   // Weak
// __issignaling
// __issignalingf
// _ITM_deregisterTMCloneTable // Weak
// _ITM_registerTMCloneTable   // Weak
GO(j0, dEd)
GO(j0f, fEf)
// __j0f_finite
// __j0_finite
// j0l
GO(j1, dEd)
GO(j1f, fEf)
// __j1f_finite
// __j1_finite
// j1l
GO(jn, dEid)
GO(jnf, fEif)
// __jnf_finite
// __jn_finite
// jnl
GOW(ldexp, dEdi)
GOW(ldexpf, fEfi)
#ifdef HAVE_LD80BITS
GOW(ldexpl, DED)
#else
GOW2(ldexpl, KEK, ldexp)
#endif
GOW(lgamma, dEd)
GOW(lgammaf, fEf)
GOW(lgammaf_r, fEfp)
// __lgammaf_r_finite
#ifdef HAVE_LD80BITS
GOW(lgammal, DED)
#else
GOW2(lgammal, KEK, lgamma)
#endif
#ifdef HAVE_LD80BITS
GOW(lgammal_r, DEDp)
#else
GOW2(lgammal_r, KEKp, lgamma_r)
#endif
GOW(lgamma_r, dEdp)
// __lgamma_r_finite
DATAV(_LIB_VERSION, 4)
GOW(llrint, IEd)
GOW(llrintf, IEf)
// llrintl  // Weak
GOW(llround, IEd)
GOW(llroundf, IEf)
// llroundl // Weak
GOW(log, dEd)
GOW(log10, dEd)
GOW(log10f, fEf)
GOM(__log10f_finite, fEf) //%noE
GOM(__log10_finite, dEd)  //%noE
// log10l   // Weak
GOW(log1p, dEd)
GOW(log1pf, fEf)
// log1pl   // Weak
GOW(log2, dEd)
GOW(log2f, fEf)
GOM(__log2f_finite, fEf) //%noE
GOM(__log2_finite, dEd)  //%noE
// log2l    // Weak
GOW(logb, dEd)
GOW(logbf, fEf)
// logbl    // Weak
GOW(logf, fEf)
GOM(__logf_finite, fEf) //%noE
GOM(__log_finite, dEd)  //%noE
#ifdef HAVE_LD80BITS
GOW(logl, DED)
#else
GOW2(logl, KEK, log)
#endif
GOW(lrint, iEd)
GOW(lrintf, iEf)
// lrintl   // Weak
GOW(lround, iEd)
GOW(lroundf, iEf)
// lroundl  // Weak
// matherr  // Weak
GOW(modf, dEdp)
GOW(modff, fEfp)
// modfl    // Weak
// nan  // Weak
// nanf // Weak
// nanl // Weak
GOW(nearbyint, dEd)
GOW(nearbyintf, fEf)
// nearbyintl   // Weak
GOW(nextafter, dEdd)
GOW(nextafterf, fEff)
// nextafterl   // Weak
GOW(nexttoward, dEdD)
GOW(nexttowardf, fEfD)
// nexttowardl  // Weak
GOW(pow, dEdd)
GOW(pow10, dEd)
GOW(pow10f, fEf)
#ifdef HAVE_LD80BITS
GOWM(pow10l, DED)   //%noE
#else
GOWM(pow10l, KEK)   //%noE
#endif
GOW(powf, fEff)
GOM(__powf_finite, fEff) //%noE
GOM(__pow_finite, dEdd)  //%noE
#ifdef HAVE_LD80BITS
GOW(powl, DEDD)
#else
GOW2(powl, KEKK, pow)
#endif
GOW(remainder, dEdd)
GOW(remainderf, fEff)
// __remainderf_finite
// __remainder_finite
// remainderl   // Weak
GOW(remquo, dEddp)
GOW(remquof, fEffp)
// remquol  // Weak
GOW(rint, dEd)
GOW(rintf, fEf)
// rintl    // Weak
GOW(round, dEd)
GOW(roundf, fEf)
// roundl   // Weak
// scalb    // Weak
// scalbf   // Weak
// __scalbf_finite
// __scalb_finite
// scalbl   // Weak
GOW(scalbln, dEdi)
GOW(scalblnf, fEfi)
// scalblnl // Weak
GOW(scalbn, dEdi)
GOW(scalbnf, fEfi)
// scalbnl  // Weak
// __signbit
// __signbitf
DATAB(signgam, 4)
// significand  // Weak
// significandf // Weak
// significandl // Weak
GOW(sin, dEd)
GOW(sincos, vEdpp)
GOW(sincosf, vEfpp)
// sincosl  // Weak
GOW(sinf, fEf)
GOW(sinh, dEd)
GOW(sinhf, fEf)
GOM(__sinhf_finite, fEf) //%noE
GOM(__sinh_finite, dEd)  //%noE
// sinhl    // Weak
// sinl // Weak
GOW(sqrt, dEd)
GOW(sqrtf, fEf)
GOM(__sqrtf_finite, fEf) //%noE
GOM(__sqrt_finite, dEd)  //%noE
// sqrtl    // Weak
GO(tan, dEd)
GOW(tanf, fEf)
GOW(tanh, dEd)
GOW(tanhf, fEf)
// tanhl    // Weak
// tanl // Weak
GOW(tgamma, dEd)
GOW(tgammaf, fEf)
#ifdef HAVE_LD80BITS
GOW(tgammal, DED)
#else
GOW2(tgammal, KEK, tgamma)
#endif
GOW(trunc, dEd)
GOW(truncf, fEf)
// truncl   // Weak
GO(y0, dEd)
GO(y0f, fEf)
// __y0f_finite
// __y0_finite
// y0l
GO(y1, dEd)
GO(y1f, fEf)
// __y1f_finite
// __y1_finite
// y1l
GO(yn, dEid)
GO(ynf, fEif)
// __ynf_finite
// __yn_finite
// ynl
