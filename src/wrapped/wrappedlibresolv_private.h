#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh...
#endif

GO(__b64_ntop, iFpLpL)
GO(__b64_pton, iFppL)
GO(__dn_comp, iFppipp)
GO(__dn_count_labels, iFp)
GOM(__dn_expand, iFEppppi)
GOM(dn_expand, iFEppppi)
GO(__dn_skipname, iFpp)
GO(__fp_nquery, vFpiS)
GO(__fp_query, vFpS)
GO(__fp_resstat, vFpS)
//GO(_gethtbyaddr, 
//GO(_gethtbyname, 
//GO(_gethtbyname2, 
//GO(_gethtent, 
//GO(_getlong, 
//GO(_getshort, 
GO(__hostalias, pFp)
GO(inet_neta, pFupL)
GO(inet_net_ntop, pFipipL)
GO(inet_net_pton, iFippL)
GO(__loc_aton, iFpp)
GO(__loc_ntoa, pFpp)
GO(ns_datetosecs, uFpp)
GO(ns_format_ttl, iFLpL)
GO(__ns_get16, uFp)
GOW(ns_get16, uFp)
GO(__ns_get32, LFp)
GOW(ns_get32, LFp)
GO(ns_initparse, iFpip)
GO(ns_makecanon, iFppL)
//GO(ns_msg_getflag, 
GO(ns_name_compress, iFppLpp)
GO(ns_name_ntol, iFppL)
GO(__ns_name_ntop, iFppL)
GOW(ns_name_ntop, iFppL)
GO(ns_name_pack, iFppipp)
GO(ns_name_pton, iFppL)
GO(ns_name_rollback, vFppp)
GO(ns_name_skip, iFpp)
GO(ns_name_uncompress, iFppppL)
GO(__ns_name_unpack, iFppppL)
GOW(ns_name_unpack, iFppppL)
GO(ns_parserr, iFpuip)
GO(ns_parse_ttl, iFpp)
GO(ns_put16, vFup)
GO(ns_put32, vFLp)
GO(ns_samedomain, iFpp)
GO(ns_samename, iFpp)
GO(ns_skiprr, iFppui)
GO(ns_sprintrr, iFpppppL)
GO(ns_sprintrrf, iFpLpuuLpLpppL)
GO(ns_subdomain, iFpp)
GO(__p_cdname, pFppS)
GO(__p_cdnname, pFppiS)
GO(__p_class, pFi)
//DATA(__p_class_syms, 4)
GO(__p_fqname, pFppS)
GO(__p_fqnname, pFppipi)
GO(__p_option, pFL)
GO(__p_query, vFp)
GO(__p_rcode, pFi)
//GO(__p_secstodate, 
GO(__p_time, pFu)
GO(__p_type, pFi)
//DATA(__p_type_syms, 4)
GO(__putlong, vFup)
GO(__putshort, vFWp)
GO(__res_close, vFv)
//GO(__res_context_hostalias, 
//GO(__res_context_query, 
//GO(__res_context_search, 
GO(__res_dnok, iFp)
//GO(res_gethostbyaddr, 
//GO(res_gethostbyname, 
//GO(res_gethostbyname2, 
GO(__res_hnok, iFp)
GO(__res_hostalias, pFpppL)
GO(__res_isourserver, iFp)
GO(__res_mailok, iFp)
GOM(__res_mkquery, iFEipiipippi)
GOM(res_mkquery, iFEipiipippi)
GO(__res_nameinquery, iFpiipp)
#ifdef STATICBUILD
//GO(__res_nmkquery,
#else
GO(__res_nmkquery, iFpipiipippi)
#endif
GO(res_nmkquery, iFpipiipippi)
GOM(__res_nquery, iFEppiipi)
GOM(res_nquery, iFEppiipi)
GOM(__res_nquerydomain, iFEpppiipi)
GOM(res_nquerydomain, iFEpppiipi)
GO(__res_nsearch, iFppiipi)
#ifdef STATICBUILD
//GO(__res_nsend,
#else
GO(__res_nsend, iFppipi)
#endif
GO(res_nsend, iFppipi)
//DATA(_res_opcodes, 4)
GO(__res_ownok, iFp)
GO(__res_queriesmatch, iFpppp)
GOM(__res_query, iFEpiipi)
GOM(res_query, iFEpiipi)
GOM(__res_querydomain, iFEppiipi)
GOM(res_querydomain, iFEppiipi)
GOM(res_search, iFEpiipi)
GOM(__res_search, iFEpiipi)
GO(__res_send, iFpipi)
//GO(res_send_setqhook, 
//GO(res_send_setrhook, 
//GO(_sethtent, 
GO(__sym_ntop, pFpip)
GO(__sym_ntos, pFpip)
GO(__sym_ston, iFppp)
