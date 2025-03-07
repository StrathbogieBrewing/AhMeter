podman run \
 --name influxdb \
 --publish 8086:8086 \
 --mount type=volume,source=influxdb2-data,target=/var/lib/influxdb2 \
 --mount type=volume,source=influxdb2-config,target=/etc/influxdb2 \
 --env DOCKER_INFLUXDB_INIT_MODE=setup \
 --env DOCKER_INFLUXDB_INIT_USERNAME=username \
 --env DOCKER_INFLUXDB_INIT_PASSWORD=password \
 --env DOCKER_INFLUXDB_INIT_ORG=magiltan \
 --env DOCKER_INFLUXDB_INIT_BUCKET=shed \
 docker.io/library/influxdb:2