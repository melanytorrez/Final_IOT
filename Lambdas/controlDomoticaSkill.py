# -*- coding: utf-8 -*-
import logging
import json
import boto3

# ======================================================
# BLOQUE DE IMPORT ÚNICO Y CORRECTO
# ======================================================
from ask_sdk_core.skill_builder import SkillBuilder
from ask_sdk_core.dispatch_components import AbstractRequestHandler, AbstractExceptionHandler
from ask_sdk_core.handler_input import HandlerInput
from ask_sdk_core.utils import is_request_type, is_intent_name
from ask_sdk_model import Response

# -------------------------------
# CONFIGURACIÓN AWS
# -------------------------------
iot_client = boto3.client('iot-data')
dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('UserThingMapping')
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# -------------------------------
# FUNCIONES AUXILIARES
# -------------------------------
def get_thing_name(user_id, lugar):
    try:
        response = table.get_item(Key={'user_id': user_id})
        if "Item" in response and "things" in response["Item"]:
            return response["Item"]["things"].get(lugar.lower(), None)
    except Exception as e:
        logger.error(f"Error al consultar DynamoDB: {e}")
    return None

def map_dispositivo(dispositivo, lugar):
    dispositivo = dispositivo.lower()
    lugar = lugar.lower()
    if dispositivo == "luz": return f"luz_{lugar}"
    if dispositivo == "puerta": return f"puerta_{lugar}"
    if dispositivo == "ventana": return f"ventana_{lugar}"
    if dispositivo == "movimiento": return f"movimiento_{lugar}"
    return None

def map_accion(accion):
    accion = accion.lower()
    if accion in ["encender", "activar"]: return "ON"
    if accion in ["apagar", "desactivar"]: return "OFF"
    if accion in ["abrir"]: return "OPEN"
    if accion in ["cerrar"]: return "CLOSED"
    return None

# ======================================================
# HANDLERS DE INTENCIONES PERSONALIZADAS
# ======================================================
class ControlarDispositivoIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("ControlarDispositivoIntent")(handler_input)

    def handle(self, handler_input):
        slots = handler_input.request_envelope.request.intent.slots
        user_id = handler_input.request_envelope.session.user.user_id
        
        accion = slots["accion"].value if "accion" in slots and slots["accion"].value else None
        dispositivo = slots["dispositivo"].value if "dispositivo" in slots and slots["dispositivo"].value else None
        lugar = slots["lugar"].value if "lugar" in slots and slots["lugar"].value else "sala"
        
        thing_name = get_thing_name(user_id, lugar)
        if thing_name is None:
            return handler_input.response_builder.speak(f"No encuentro un dispositivo asociado para el lugar {lugar}.").response

        atributo = map_dispositivo(dispositivo, lugar)
        valor = map_accion(accion)
        
        if atributo is None or valor is None or accion is None or dispositivo is None:
            return handler_input.response_builder.speak("No entendí bien qué dispositivo o acción quieres realizar.").response

        payload = {"state": {"desired": {atributo: valor}}}
        try:
            iot_client.update_thing_shadow(thingName=thing_name, payload=json.dumps(payload))
            speak_output = f"Ok, voy a {accion} la {dispositivo} de la {lugar}."
        except Exception as e:
            logger.error(f"Error al actualizar el Shadow: {e}")
            speak_output = f"Tuve un problema al intentar controlar la {dispositivo} de la {lugar}."

        return handler_input.response_builder.speak(speak_output).ask("¿Necesitas algo más?").response

class EstadoDispositivoIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("EstadoDispositivoIntent")(handler_input)

    def handle(self, handler_input):
        slots = handler_input.request_envelope.request.intent.slots
        user_id = handler_input.request_envelope.session.user.user_id
        
        dispositivo = slots["dispositivo"].value if "dispositivo" in slots and slots["dispositivo"].value else "movimiento"
        lugar = slots["lugar"].value if "lugar" in slots and slots["lugar"].value else "sala"

        thing_name = get_thing_name(user_id, lugar)
        if thing_name is None:
            return handler_input.response_builder.speak(f"No encuentro un dispositivo asociado para el lugar {lugar}.").response
            
        atributo = map_dispositivo(dispositivo, lugar)
        if atributo is None:
            return handler_input.response_builder.speak(f"No conozco el dispositivo {dispositivo}.").response

        try:
            response = iot_client.get_thing_shadow(thingName=thing_name)
            shadow = json.loads(response["payload"].read().decode('utf-8'))
            estado = shadow.get("state", {}).get("reported", {}).get(atributo, "desconocido")
            speak_output = f"El estado de la {dispositivo} de la {lugar} es {estado}."
        except Exception as e:
            logger.error(f"Error al obtener el Shadow: {e}")
            speak_output = "No pude obtener el estado del dispositivo."

        return handler_input.response_builder.speak(speak_output).ask("¿Necesitas algo más?").response

# ======================================================
# HANDLERS ESTÁNDAR DE ALEXA
# ======================================================
class LaunchRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_request_type("LaunchRequest")(handler_input)
    def handle(self, handler_input):
        return handler_input.response_builder.speak("Bienvenido al sistema inteligente. ¿Qué deseas hacer?").ask("Puedes decir: enciende la luz de la sala.").response

class HelpIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("AMAZON.HelpIntent")(handler_input)
    def handle(self, handler_input):
        return handler_input.response_builder.speak("Puedes dar órdenes como: abre la puerta de la cocina.").ask("¿Qué deseas hacer?").response

class CancelOrStopIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("AMAZON.CancelIntent")(handler_input) or is_intent_name("AMAZON.StopIntent")(handler_input)
    def handle(self, handler_input):
        return handler_input.response_builder.speak("Adiós.").response

class FallbackIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return is_intent_name("AMAZON.FallbackIntent")(handler_input)
    def handle(self, handler_input):
        return handler_input.response_builder.speak("No entendí tu orden. Intenta con: enciende la luz de la sala.").ask("¿Qué deseas hacer?").response

class CatchAllExceptionHandler(AbstractExceptionHandler):
    def can_handle(self, handler_input, exception):
        return True
    def handle(self, handler_input, exception):
        logger.error(exception, exc_info=True)
        return handler_input.response_builder.speak("Ocurrió un error procesando tu solicitud.").ask("¿Deseas intentar otra cosa?").response

# ======================================================
# CONSTRUCCIÓN DE LA SKILL
# ======================================================
sb = SkillBuilder()
sb.add_request_handler(LaunchRequestHandler())
sb.add_request_handler(ControlarDispositivoIntentHandler())
sb.add_request_handler(EstadoDispositivoIntentHandler())
sb.add_request_handler(HelpIntentHandler())
sb.add_request_handler(CancelOrStopIntentHandler())
sb.add_request_handler(FallbackIntentHandler())
sb.add_exception_handler(CatchAllExceptionHandler())

lambda_handler = sb.lambda_handler()