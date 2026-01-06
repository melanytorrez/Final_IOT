import json
import boto3

iot_client = boto3.client('iot-data')
THING_NAME = 'ControlSala' # El Thing que queremos controlar

def lambda_handler(event, context):
    print(f"Función activada por la regla de movimiento. Evento: {json.dumps(event)}")
    
    # El payload que queremos enviar al Shadow
    payload = {
        "state": {
            "desired": {
                "luz_sala": "ON"
            }
        }
    }
    
    try:
        # Publicamos la actualización en el Shadow del Thing correcto
        iot_client.update_thing_shadow(
            thingName=THING_NAME,
            payload=json.dumps(payload)
        )
        print("Actualización del Shadow enviada correctamente.")
        return {'statusCode': 200, 'body': 'Shadow update sent!'}
    
    except Exception as e:
        print(f"Error al actualizar el Shadow: {e}")
        raise e