/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Bernhard Boser
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>

#include "shared-bindings/atomic/FIFO2.h"
#include "shared-bindings/util.h"

#include "lib/utils/context_manager_helpers.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "py/runtime0.h"

//| .. currentmodule:: atomic
//|
//| :class:`FIFO2` -- Interrupt safe FIFO
//| ====================================================================================
//|
//| Wrapper for nRF5 SDK v15.2.0 Atomic FIFO.
//| Consult documentation at https://infocenter.nordicsemi.com
//| (nRF5 v15.2.0 SDK --> Libraries --> AtomicFIFO) for capabilities and limitations.
//|
//| Depreceated. Will be removed in a future release.
//|
//| .. class:: FIFO2(size)
//|
//|   Create FIFO of specified fixed size.
//|
//|   :param ~int size
//|
STATIC mp_obj_t atomic_fifo2_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check number of arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    mp_int_t size = mp_obj_get_int(args[0]);

    atomic_fifo2_obj_t *self = m_new_obj(atomic_fifo2_obj_t);
    self->base.type = &atomic_fifo2_type;
    common_hal_atomic_fifo2_construct(self, size);
    return (mp_obj_t)self;
}

//|   .. method:: put
//|
//|     Add item. Raises IndexError if queue is full.
//|
STATIC mp_obj_t atomic_fifo2_obj_put(mp_obj_t self_in, mp_obj_t item1, mp_obj_t item2) {
    atomic_fifo2_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_atomic_fifo2_put(self, item1, item2);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(atomic_fifo2_put_obj, atomic_fifo2_obj_put);

//|   .. method:: get
//|
//|     Get item from fifo.
//|     item_list is a list of length 2 (at least).
//|
//|     :param ~list item_list
//|
//|     :exception raises IndexError if empty.
//|
STATIC mp_obj_t atomic_fifo2_obj_get(mp_obj_t self_in, mp_obj_t item_list) {
    atomic_fifo2_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_list_t *ret = MP_OBJ_TO_PTR(item_list);
    if (!MP_OBJ_IS_TYPE(item_list, &mp_type_list) || ret->len < 2) {
        mp_raise_TypeError(translate("expects list"));
    }

    common_hal_atomic_fifo2_get(self, ret);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(atomic_fifo2_get_obj, atomic_fifo2_obj_get);

STATIC const mp_rom_map_elem_t atomic_fifo2_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_put),    MP_ROM_PTR(&atomic_fifo2_put_obj) },
    { MP_ROM_QSTR(MP_QSTR_get),    MP_ROM_PTR(&atomic_fifo2_get_obj) },
};
STATIC MP_DEFINE_CONST_DICT(atomic_fifo2_locals_dict, atomic_fifo2_locals_dict_table);

const mp_obj_type_t atomic_fifo2_type = {
    { &mp_type_type },
    .name = MP_QSTR_FIFO2,
    .make_new = atomic_fifo2_make_new,
    .locals_dict = (mp_obj_dict_t*)&atomic_fifo2_locals_dict,
};
