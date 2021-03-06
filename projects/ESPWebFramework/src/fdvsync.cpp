/*
# Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
# Copyright (c) 2015 Fabrizio Di Vittorio.
# All rights reserved.

# GNU GPL LICENSE
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; latest version thereof,
# available at: <http://www.gnu.org/licenses/gpl.txt>.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/


#include "fdv.h"




extern "C" void ets_wdt_enable();
extern "C" void ets_wdt_disable();
extern "C" void ets_intr_lock();
extern "C" void ets_intr_unlock();



namespace fdv
{

	
    
	void FUNC_FLASHMEM enterCritical()
	{
		taskENTER_CRITICAL();
	}
	
    
	void FUNC_FLASHMEM exitCritical()
	{
		taskEXIT_CRITICAL();
	}
    
	
	uint32_t FUNC_FLASHMEM millisISR()
	{
		return xTaskGetTickCountFromISR() * portTICK_RATE_MS;
	}
	
	
	uint32_t FUNC_FLASHMEM millis()
	{
		return xTaskGetTickCount() * portTICK_RATE_MS; 
	}
	
	
	// calculates time difference in milliseconds, taking into consideration the time overflow
	// note: time1 must be less than time2 (time1 < time2)
	uint32_t FUNC_FLASHMEM millisDiff(uint32_t time1, uint32_t time2)
	{
		if (time1 > time2)
			// overflow
			return 0xFFFFFFFF - time1 + time2;
		else
			return time2 - time1;
	}
	
    
    uint32_t FUNC_FLASHMEM micros()
    {
        return system_get_time();
    }
    
	
	/////////////////////////////////////////////////////////////////////+
	/////////////////////////////////////////////////////////////////////+
	// Mutex

    MTD_FLASHMEM Mutex::Mutex()
        : m_handle(NULL)
    {
        vSemaphoreCreateBinary(m_handle);
    }
    
    MTD_FLASHMEM Mutex::~Mutex()
    {
        vSemaphoreDelete(m_handle);
    }
    
    bool MTD_FLASHMEM Mutex::lock(uint32_t msTimeOut)
    {
        return xSemaphoreTake(m_handle, msTimeOut / portTICK_RATE_MS);
    }
    
    bool MTD_FLASHMEM Mutex::lockFromISR()
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        return xSemaphoreTakeFromISR(m_handle, &xHigherPriorityTaskWoken);
    }
    
    void MTD_FLASHMEM Mutex::unlock()
    {
        xSemaphoreGive(m_handle);
    }
    
    void MTD_FLASHMEM Mutex::unlockFromISR()
    {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(m_handle, &xHigherPriorityTaskWoken);				
    }
    
	
	/////////////////////////////////////////////////////////////////////+
	/////////////////////////////////////////////////////////////////////+
	// MutexLock & MutexLockFromISR

    MTD_FLASHMEM MutexLock::MutexLock(Mutex* mutex, uint32_t msTimeOut)
      : m_mutex(mutex)
    {
        m_acquired = m_mutex->lock(msTimeOut);
    }

    MTD_FLASHMEM MutexLock::~MutexLock()
    {
        if (m_acquired)
            m_mutex->unlock();
    }

    
    MTD_FLASHMEM MutexLockFromISR::MutexLockFromISR(Mutex* mutex)
      : m_mutex(mutex)
    {
        m_acquired = m_mutex->lockFromISR();
    }

    MTD_FLASHMEM MutexLockFromISR::~MutexLockFromISR()
    {
        if (m_acquired)
            m_mutex->unlockFromISR();
    }

    
 
	/////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
	// SoftTimeOut class

    MTD_FLASHMEM SoftTimeOut::SoftTimeOut(uint32_t time)
        : m_timeOut(time), m_startTime(millis())
    {
    }

    
    MTD_FLASHMEM SoftTimeOut::operator bool()
    {
        return millisDiff(m_startTime, millis()) > m_timeOut;
    }
    
    
    void MTD_FLASHMEM SoftTimeOut::reset(uint32_t time)
    {
        m_timeOut   = time;
        m_startTime = millis();				
    }
    
 
}