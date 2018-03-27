Práctica 2 sistemas operativos I

INTRODUCCIÓN
-------------

Este programa simula el funcionamiento del shell. Sus comandos internos son cd,jobs,export,source,fg,bg y exit. Admite procesos en segundo plano, redireccionamiento, manejo de señales para los manejadores CTRL-C,CTRL-Z y el uso de comandos externos.
-------------------------------------------------------------------

MEJORAS REALIZADAS
-------------------
*** En las fases finales se han introducido mejoras que no constan en las iniciales,
*** la última fase contiene todas las mejoras y cambios realizados.
	- Cambios en el read_line
	- Colores
	- Cambios en la sintáxis del prompt
	- ...
*** Extras del MINISHELL:
	- uso variables de entorno en el prompt y colores
	- tratamiento de ctrl-Z
	- visualización del comando + argumentos + estado
	- implementación comandos internos fg
	- implementación comandos internos bg
	- edicion avanzada de la línea de comandos e historial
------------------------------------------------------------------

RESTRICCIONES DEL PROGRAMA
---------------------------

- En el uso de ctrl-c y ctrl-z unicamente, sin haber ningun proceso ejecutandose no aparce el prompt, hemos de darle a enter para que aparezca denuevo.
- Al lanzar el proceso sleep NUM & en segundo plano, al terminar, el mensaje que indica la finalización aparece antes del propmt, teniendo que pulsar enter para que vuelva a aparecer.
- en el redireccionamiento con '>' es necesario dejar el símbolo separado por espacios entre el comando a redireccionar y el fichero.
------------------------------------------------------------------

SINTAXIS ESPECÍFICA
--------------------

- todos los comandos han de escribirse en minúsculas

------------------------------------------------------------------

============================================
Miembros del grupo:

- Luis Arjona Ruiz
- Hector Alberto Santos Rodriguez
