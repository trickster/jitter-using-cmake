/* Izmir langauge: AST operations.

   Copyright (C) 2019 Luca Saiu
   Copyright (C) 2022 Luca Saiu
   Written by Luca Saiu

   This file is part of the İzmir example, based on the Jitter
   structured-language example which is distributed along with
   GNU Jitter under its same license.

   GNU Jitter is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GNU Jitter is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Jitter.  If not, see <http://www.gnu.org/licenses/>. */


#include <stdbool.h>

#include "izmir-syntax.h"


/* Reversing of boolean primitives.
 * ************************************************************************** */

bool
izmir_is_comparison_primitive (enum izmir_primitive p)
{
  switch (p)
    {
    case izmir_primitive_equal:
    case izmir_primitive_different:
    case izmir_primitive_less:
    case izmir_primitive_less_or_equal:
    case izmir_primitive_greater:
    case izmir_primitive_greater_or_equal:
    case izmir_primitive_logical_not:
    case izmir_primitive_is_nonzero:
      return true;
    default:
      return false;
    }
}

enum izmir_primitive
izmir_reverse_comparison_primitive (enum izmir_primitive p)
{
  switch (p)
    {
    case izmir_primitive_equal:
      return izmir_primitive_different;
    case izmir_primitive_different:
      return izmir_primitive_equal;
    case izmir_primitive_less:
      return izmir_primitive_greater_or_equal;
    case izmir_primitive_less_or_equal:
      return izmir_primitive_greater;
    case izmir_primitive_greater:
      return izmir_primitive_less_or_equal;
    case izmir_primitive_greater_or_equal:
      return izmir_primitive_less;
    case izmir_primitive_logical_not:
      return izmir_primitive_is_nonzero;
    case izmir_primitive_is_nonzero:
      return izmir_primitive_logical_not;
    default:
      jitter_fatal ("cannot reverse boolean (?) primitive: %i", (int) p);
    }
}
