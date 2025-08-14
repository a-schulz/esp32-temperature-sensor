-- TimescaleDB initialization script
-- This script runs automatically when the container starts for the first time

-- Create TimescaleDB extension
CREATE EXTENSION IF NOT EXISTS timescaledb;

-- Function to create hypertables and dependent views after Home Assistant creates the tables
CREATE OR REPLACE FUNCTION create_ha_hypertables()
RETURNS void AS $$
DECLARE
    has_states boolean;
    has_attributes_col boolean;
    has_attributes_table boolean;
    has_attributes_shared boolean;
    has_last_updated_ts boolean;
    has_statistics boolean;
BEGIN
    -- Create hypertables for Home Assistant tables (will be created later)
    has_states := EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'states');
    IF has_states THEN
        IF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'states' AND column_name = 'last_updated_ts'
        ) THEN
            PERFORM create_hypertable('states', 'last_updated_ts', if_not_exists => TRUE);
        ELSIF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'states' AND column_name = 'last_updated'
        ) THEN
            PERFORM create_hypertable('states', 'last_updated', if_not_exists => TRUE);
        END IF;
    END IF;
    
    IF EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'events') THEN
        IF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'events' AND column_name = 'time_fired_ts'
        ) THEN
            PERFORM create_hypertable('events', 'time_fired_ts', if_not_exists => TRUE);
        ELSIF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'events' AND column_name = 'time_fired'
        ) THEN
            PERFORM create_hypertable('events', 'time_fired', if_not_exists => TRUE);
        END IF;
    END IF;
    
    IF EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'statistics') THEN
        IF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'statistics' AND column_name = 'created_ts'
        ) THEN
            PERFORM create_hypertable('statistics', 'created_ts', if_not_exists => TRUE);
        ELSIF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'statistics' AND column_name = 'start_ts'
        ) THEN
            PERFORM create_hypertable('statistics', 'start_ts', if_not_exists => TRUE);
        ELSIF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'statistics' AND column_name = 'start'
        ) THEN
            PERFORM create_hypertable('statistics', 'start', if_not_exists => TRUE);
        END IF;
    END IF;
    
    IF EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = 'statistics_short_term') THEN
        IF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'statistics_short_term' AND column_name = 'created_ts'
        ) THEN
            PERFORM create_hypertable('statistics_short_term', 'created_ts', if_not_exists => TRUE);
        ELSIF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'statistics_short_term' AND column_name = 'start_ts'
        ) THEN
            PERFORM create_hypertable('statistics_short_term', 'start_ts', if_not_exists => TRUE);
        ELSIF EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'statistics_short_term' AND column_name = 'start'
        ) THEN
            PERFORM create_hypertable('statistics_short_term', 'start', if_not_exists => TRUE);
        END IF;
    END IF;

    -- Create a view for easier temperature data access if base tables exist
    IF has_states THEN
        has_attributes_col := EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'states' AND column_name = 'attributes'
        );
        has_attributes_table := EXISTS (
            SELECT 1 FROM information_schema.tables WHERE table_name = 'state_attributes'
        );
        has_attributes_shared := EXISTS (
            SELECT 1 FROM information_schema.tables WHERE table_name = 'state_attributes_shared'
        );
        has_last_updated_ts := EXISTS (
            SELECT 1 FROM information_schema.columns
            WHERE table_name = 'states' AND column_name = 'last_updated_ts'
        );

        IF has_attributes_col THEN
            -- Old schema with JSON attributes column directly on states
            EXECUTE $$
            CREATE OR REPLACE VIEW temperature_view AS
            SELECT
                $$ || CASE WHEN has_last_updated_ts THEN 'last_updated_ts' ELSE 'last_updated' END || $$ as time,
                entity_id,
                state::numeric as temperature,
                attributes->>'unit_of_measurement' as unit,
                COALESCE(attributes->>'friendly_name', entity_id) as location
            FROM states
            WHERE entity_id LIKE 'sensor.%temperature%'
              AND state ~ '^-?[0-9]+\.?[0-9]*$'
            ORDER BY 1 DESC
            $$;
        ELSIF has_attributes_table THEN
            -- Schema with separate state_attributes table
            EXECUTE $$
            CREATE OR REPLACE VIEW temperature_view AS
            SELECT
                $$ || CASE WHEN has_last_updated_ts THEN 's.last_updated_ts' ELSE 's.last_updated' END || $$ as time,
                s.entity_id,
                s.state::numeric as temperature,
                sa.attributes->>'unit_of_measurement' as unit,
                COALESCE(sa.attributes->>'friendly_name', s.entity_id) as location
            FROM states s
            JOIN state_attributes sa ON sa.attributes_id = s.attributes_id
            WHERE s.entity_id LIKE 'sensor.%temperature%'
              AND s.state ~ '^-?[0-9]+\.?[0-9]*$'
            ORDER BY 1 DESC
            $$;
        ELSIF has_attributes_shared THEN
            -- Newer schema with shared attributes
            EXECUTE $$
            CREATE OR REPLACE VIEW temperature_view AS
            SELECT
                $$ || CASE WHEN has_last_updated_ts THEN 's.last_updated_ts' ELSE 's.last_updated' END || $$ as time,
                s.entity_id,
                s.state::numeric as temperature,
                sas.shared_attrs->>'unit_of_measurement' as unit,
                COALESCE(sas.shared_attrs->>'friendly_name', s.entity_id) as location
            FROM states s
            JOIN state_attributes_shared sas ON sas.shared_attrs_id = s.attributes_id
            WHERE s.entity_id LIKE 'sensor.%temperature%'
              AND s.state ~ '^-?[0-9]+\.?[0-9]*$'
            ORDER BY 1 DESC
            $$;
        END IF;

        -- Grant access to the view if grafana role exists
        IF EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'grafana') THEN
            EXECUTE 'GRANT SELECT ON temperature_view TO grafana';
        END IF;
    END IF;
END;
$$ LANGUAGE plpgsql;
