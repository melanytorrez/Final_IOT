import json
import boto3

# Inicializar clientes de AWS
dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('ValidRfidCards')
iot_client = boto3.client('iot-data')

def lambda_handler(event, context):
    print(f"Evento recibido de la regla de IoT: {json.dumps(event)}")

    # 1. Extraer el UID de la tarjeta y el nombre del 'Thing' del evento
    # Gracias a la regla SQL, estos datos ya vienen en el diccionario 'event'.
    card_uid = event.get('card_uid')
    thing_name = event.get('thing_name')

    # Validación de entrada
    if not card_uid or not thing_name:
        print(f"Error: 'card_uid' o 'thing_name' no encontrados en el evento: {json.dumps(event)}")
        return {'statusCode': 400, 'body': 'Payload de entrada inválido'}
    
    print(f"UID de tarjeta extraído: {card_uid} para el Thing: {thing_name}")
        
    # 2. Lógica de Validación en DynamoDB
    response_status = "INVALID"
    try:
        response = table.get_item(Key={'card_uid': card_uid})
        if 'Item' in response:
            response_status = "VALID"
        print(f"Resultado de la validación en DynamoDB: {response_status}")
            
    except Exception as e:
        print(f"Error al consultar DynamoDB: {e}")
        response_status = "INVALID"

    # 3. Construir el tema de respuesta DINÁMICAMENTE
    response_topic = f'{thing_name}/rfid/checkResponse'
    
    response_payload = {
        'status': response_status,
        'card_uid': card_uid
    }
    
    # 4. Publicar la respuesta en el tema correcto
    try:
        iot_client.publish(
            topic=response_topic,
            qos=1,
            payload=json.dumps(response_payload)
        )
        print(f"Publicada respuesta en {response_topic}: {json.dumps(response_payload)}")
    except Exception as e:
        print(f"Error al publicar en AWS IoT: {e}")

    return {
        'statusCode': 200,
        'body': json.dumps(f'Procesada tarjeta {card_uid} para {thing_name}, resultado: {response_status}')
    }