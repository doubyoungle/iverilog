/*
 * Copyright (c) 2002-2009 Michael Ruff (mruff at chiaro.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include  <veriuser.h>
#include  <vpi_user.h>
#include  <stdlib.h>
#include  <math.h>
#include  "config.h"
#include  "priv.h"
#include  <assert.h>

/*
 * some TF time routines implemented using VPI interface
 */

static ivl_u64_t
scale(int high, int low, void*obj) {
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));
      ivl_u64_t scaled;

      scaled = high;
      scaled = (scaled << 32) | low;
      scaled *= pow(10, vpi_get(vpiTimePrecision,0) -
			vpi_get(vpiTimeUnit,obj ? (vpiHandle)obj : hand));

      return scaled;
}


PLI_INT32 tf_gettime(void)
{
      s_vpi_time time;
      time.type = vpiSimTime;
      vpi_get_time (0, &time);
      return scale(time.high, time.low, 0) & 0xffffffff;
}

char *tf_strgettime(void)
{
      static char buf[32];
      s_vpi_time time;

      time.type = vpiSimTime;
      vpi_get_time (0, &time);
      if (time.high)
	    snprintf(buf, sizeof(buf)-1, "%u%08u", (unsigned int)time.high,
	             (unsigned int)time.low);
      else
	    snprintf(buf, sizeof(buf)-1, "%u", (unsigned int)time.low);
      return buf;
}

PLI_INT32 tf_igetlongtime(PLI_INT32 *high, void*obj)
{
      s_vpi_time time;
      ivl_u64_t scaled;
      time.type = vpiSimTime;
      vpi_get_time ((vpiHandle)obj, &time);
      scaled = scale(time.high, time.low, obj);

      *high = (scaled >> 32) & 0xffffffff;
      return scaled & 0xffffffff;
}

PLI_INT32 tf_getlongtime(PLI_INT32 *high)
{
      return tf_igetlongtime(high, 0);
}

/* Alias for commercial simulators */
PLI_INT32 tf_getlongsimtime(PLI_INT32 *high) \
      __attribute__ ((weak, alias ("tf_getlongtime")));

void tf_scale_longdelay(void*obj, PLI_INT32 low, PLI_INT32 high,
			PLI_INT32 *alow, PLI_INT32 *ahigh)
{
      ivl_u64_t scaled = scale(high, low, obj);
      *ahigh = (scaled >> 32) & 0xffffffff;
      *alow = scaled & 0xffffffff;
}

void tf_unscale_longdelay(void*obj, PLI_INT32 low, PLI_INT32 high,
			  PLI_INT32 *alow, PLI_INT32 *ahigh)
{
      ivl_u64_t unscaled;
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));

      unscaled = high;
      unscaled = (unscaled << 32) | low;
      unscaled *= pow(10, vpi_get(vpiTimeUnit, hand) -
			  vpi_get(vpiTimePrecision, 0));

      *ahigh = (unscaled >> 32) & 0xffffffff;
      *alow = unscaled & 0xffffffff;
}

void tf_scale_realdelay(void*obj, double real, double *areal)
{
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));

      *areal = real * pow(10, vpi_get(vpiTimePrecision, 0) -
			      vpi_get(vpiTimeUnit, hand));
}

void tf_unscale_realdelay(void*obj, double real, double *areal)
{
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));

      *areal = real * pow(10, vpi_get(vpiTimeUnit, hand) -
			      vpi_get(vpiTimePrecision, 0));
}

PLI_INT32 tf_gettimeprecision(void)
{
      PLI_INT32 rc;
      vpiHandle hand;
      vpiHandle sys = vpi_handle(vpiSysTfCall,0);
      assert(sys);

      hand = vpi_handle(vpiScope, sys);
      rc = vpi_get(vpiTimePrecision, hand);

      if (pli_trace)
	    fprintf(pli_trace, "tf_gettimeprecision(<%s>) --> %d\n",
		    vpi_get_str(vpiName, sys), (int)rc);

      return rc;
}

PLI_INT32 tf_igettimeprecision(void*obj)
{
      PLI_INT32 rc;

      if (obj == 0) {
	      /* If the obj pointer is null, then get the simulation
		 time precision. */
	    rc = vpi_get(vpiTimePrecision, 0);

      } else {

	    vpiHandle scope = vpi_handle(vpiScope, (vpiHandle)obj);
	    assert(scope);
	    rc = vpi_get(vpiTimePrecision, scope);
      }

      if (pli_trace)
	    fprintf(pli_trace, "tf_igettimeprecision(<%s>) --> %d\n",
		    obj? vpi_get_str(vpiName, obj) : ".", (int)rc);

      return rc;
}


PLI_INT32 tf_gettimeunit()
{
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));
      return vpi_get(vpiTimeUnit, hand);
}

PLI_INT32 tf_igettimeunit(void*obj)
{
      return vpi_get(!obj ? vpiTimePrecision : vpiTimeUnit, (vpiHandle)obj);
}
