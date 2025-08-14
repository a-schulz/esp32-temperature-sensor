#!/bin/sh
set -e

# This script runs inside the TimescaleDB container on first init
# Create Grafana DB user and grant read-only permissions

: "${POSTGRES_DB:?POSTGRES_DB is required}"
: "${POSTGRES_USER:?POSTGRES_USER is required}"
: "${POSTGRES_PASSWORD:?POSTGRES_PASSWORD is required}"
: "${GRAFANA_DB_PASSWORD:?GRAFANA_DB_PASSWORD is required (pass via docker-compose env)}"

psql -v ON_ERROR_STOP=1 \
  -v grafana_pwd="$GRAFANA_DB_PASSWORD" \
  --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" <<-'EOSQL'
DO $$
BEGIN
  IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'grafana') THEN
    EXECUTE format('CREATE USER grafana WITH PASSWORD %L', :'grafana_pwd');
  END IF;
END
$$;

GRANT CONNECT ON DATABASE current_database() TO grafana;
GRANT USAGE ON SCHEMA public TO grafana;
GRANT SELECT ON ALL TABLES IN SCHEMA public TO grafana;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT SELECT ON TABLES TO grafana;
EOSQL
