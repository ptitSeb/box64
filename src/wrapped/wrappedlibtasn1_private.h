#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
error Meh...
#endif

GO(asn1_array2tree, iFppp)
GO(asn1_bit_der, iFpppp)
GO(asn1_check_version, pFp)
//GO(asn1_copy_node, 
//GO(asn1_create_element,
GO(asn1_decode_simple_ber, iFpLp)
GO(asn1_decode_simple_der, iFpLp)
GO(asn1_delete_element, iFp)
GO(asn1_delete_structure, iFp)
GO(asn1_delete_structure2, iFpp)
//GO(asn1_der_coding, 
GO(asn1_der_decoding, iFppip)
GO(asn1_der_decoding2, iFppip)
GO(asn1_der_decoding_element, iFppip)
GO(asn1_der_decoding_startEnd, iFppippp)
//GO(asn1_dup_node, 
GO(asn1_encode_simple_der, iFppppp)
//GO(asn1_expand_any_defined_by, 
//GO(asn1_expand_octet_string,
//GO(asn1_find_node,
GO(asn1_find_structure_from_oid, iFpp)
//GO(asn1_get_bit_der, 
GO(asn1_get_length_ber, iFpip)
GO(asn1_get_length_der, iFpip)
//GO(asn1_get_object_id_der, 
//GO(asn1_get_octet_der,
//GO(asn1_get_tag_der,
//GO(asn1_length_der,
//GO(asn1_number_of_elements,
GO(asn1_object_id_der, iFppp)
GO(asn1_octet_der, iFpipp)
//GO(asn1_parser2array, 
//GO(asn1_parser2tree,
GO(asn1_perror, vFpi)
//GO(asn1_print_structure, 
//GO(asn1_read_node_value,
GO(asn1_read_tag, iFpip)
//GO(asn1_read_value, 
//GO(asn1_read_value_type,
GO(asn1_strerror, pFi)
//GO(asn1_write_value,

