/*
 * mdc.h
 *
 * Internal MDC manipulation functions
 *
 *  Copyright (c) 2019 AT&T Intellectual Property.
 *  Copyright (c) 2018-2019 Nokia.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This source code is part of the near-RT RIC (RAN Intelligent Controller)
 *  platform project (RICP).
 */


#ifndef INCLUDE_PRIVATE_MDC_H_
#define INCLUDE_PRIVATE_MDC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Empty declaration for mdc_t, which is opaque to the user
 */
typedef struct mdc mdc_t;

/**
 * Init mdc subsystem
 */
int mdclog_internal_init_mdc(void);

/**
 * Get first MDC in the list for the thread
 * or null if there are no MDC's set
 *
 * @return    MDC pointer or NULL
 */
mdc_t *mdclog_internal_get_first_mdc(void);

/**
 * Add an MDC
 *
 * @param   key     The key
 * @param   value   The value
 *
 * @return   -1 in case of error. Errno is set
 */
int mdclog_internal_put_mdc(const char *key, const char *value);

/**
 * Remove an MMC
 *
 * @param    key   The key
 */
void mdclog_internal_rm_mdc(const char *key);

/**
 * Get next MDC in the list
 *
 * @param     mdc   Previous MDC, returned by mdclog_internal_get_first_mdc() or mdclog_internal_get_next_mdc()
 *
 * @return    MDC or NULL if no more MDCs
 */
mdc_t *mdclog_internal_get_next_mdc(mdc_t *mdc);

/**
 * Search an MDC with key
 *
 * @param    key   The key
 *
 * @return   MDC or NULL if MDC is not mount
 */
mdc_t *mdclog_internal_search_mdc(const char *key);

/**
 * Clear the MDC list, i.e. remove all MDCs
 */
void mdclog_internal_clean_mdclist(void);

/**
 * Get the value of an MDC
 *
 * @param   mdc   The MDC returned by mdclog_internal_get_first_mdc() or mdclog_internal_get_next_mdc()
 *
 * @return  The MDC value
 */
const char *mdclog_internal_get_mdc_val(mdc_t *mdc);

/**
 * Get the key of an MDC
 *
 * @param   mdc   The MDC returned by mdclog_internal_get_first_mdc() or mdclog_internal_get_next_mdc()
 * @return  The MDC key
 */
const char *mdclog_internal_get_mdc_key(mdc_t *mdc);

/**
 * Destroy whole MDC list, including the list pointer itself
 */
void mdclog_internal_destroy_mdclist(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_PRIVATE_MDC_H_ */
