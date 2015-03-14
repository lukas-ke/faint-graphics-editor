#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2013 Lukas Kemmer
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You
# may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

import xml.parsers.expat
from xml.parsers.expat import ExpatError

def is_document_related(err):
    """Returns True for expat errors related to errors in the
    document, which should not be reported as internal errors.

    """
    errors = xml.parsers.expat.errors
    err_str = xml.parsers.expat.ErrorString(err.code)

    # All errors in the expat.errors-module in Python 2.7.2, with
    # those that seem related to internal errors commented away.
    return err_str in (
        #errors.XML_ERROR_ABORTED,
        #errors.XML_ERROR_ASYNC_ENTITY,
        errors.XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF,
        errors.XML_ERROR_BAD_CHAR_REF,
        errors.XML_ERROR_BINARY_ENTITY_REF,
        #errors.XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING,
        errors.XML_ERROR_DUPLICATE_ATTRIBUTE,
        #errors.XML_ERROR_ENTITY_DECLARED_IN_PE,
        #errors.XML_ERROR_EXTERNAL_ENTITY_HANDLING,
        #errors.XML_ERROR_FEATURE_REQUIRES_XML_DTD,
        #errors.XML_ERROR_FINISHED,
        errors.XML_ERROR_INCOMPLETE_PE,
        errors.XML_ERROR_INCORRECT_ENCODING,
        errors.XML_ERROR_INVALID_TOKEN,
        errors.XML_ERROR_JUNK_AFTER_DOC_ELEMENT,
        errors.XML_ERROR_MISPLACED_XML_PI,
        #errors.XML_ERROR_NOT_STANDALONE,
        #errors.XML_ERROR_NOT_SUSPENDED,
        errors.XML_ERROR_NO_ELEMENTS,
        #errors.XML_ERROR_NO_MEMORY,
        errors.XML_ERROR_PARAM_ENTITY_REF,
        errors.XML_ERROR_PARTIAL_CHAR,
        errors.XML_ERROR_PUBLICID,
        errors.XML_ERROR_RECURSIVE_ENTITY_REF,
        #errors.XML_ERROR_SUSPENDED,
        #errors.XML_ERROR_SUSPEND_PE,
        errors.XML_ERROR_SYNTAX,
        errors.XML_ERROR_TAG_MISMATCH,
        #errors.XML_ERROR_TEXT_DECL,
        #errors.XML_ERROR_UNBOUND_PREFIX,
        errors.XML_ERROR_UNCLOSED_CDATA_SECTION,
        errors.XML_ERROR_UNCLOSED_TOKEN,
        #errors.XML_ERROR_UNDECLARING_PREFIX,
        errors.XML_ERROR_UNDEFINED_ENTITY,
        #errors.XML_ERROR_UNEXPECTED_STATE,
        errors.XML_ERROR_UNKNOWN_ENCODING,
        errors.XML_ERROR_XML_DECL)
