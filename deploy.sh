#!/usr/bin/env bash

# Deployment script for Home Automation Stack (Infrastructure only)
# Usage: ./deploy.sh [user@]hostname [remote_path]
# Example: ./deploy.sh pi@192.168.1.100 /home/pi/home-automation

set -e

# Configuration
REMOTE_HOST="${1:-pi@192.168.1.100}"
REMOTE_PATH="${2:-/home/pi/home-automation}"
LOCAL_PATH="$(pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 Deploying Home Automation Infrastructure to ${REMOTE_HOST}${NC}"
echo -e "${BLUE}Local path: ${LOCAL_PATH}${NC}"
echo -e "${BLUE}Remote path: ${REMOTE_PATH}${NC}"

# Check if remote host is reachable
echo -e "\n${YELLOW}📡 Testing connection to remote host...${NC}"
if ! ssh -o ConnectTimeout=5 -o BatchMode=yes "$REMOTE_HOST" exit 2>/dev/null; then
    echo -e "${RED}❌ Cannot connect to $REMOTE_HOST${NC}"
    echo -e "${RED}   Make sure SSH keys are set up or use: ssh-copy-id $REMOTE_HOST${NC}"
    exit 1
fi
echo -e "${GREEN}✅ Connection successful${NC}"

# Create remote directory
echo -e "\n${YELLOW}📁 Creating remote directory...${NC}"
ssh "$REMOTE_HOST" "mkdir -p $REMOTE_PATH"

# Sync only home automation files using rsync with better exclusions
echo -e "\n${YELLOW}📦 Syncing home automation infrastructure with rsync...${NC}"
rsync -avz --progress --delete \
    --include='homeAutomation/' \
    --include='homeAutomation/docker-compose*.yml' \
    --include='homeAutomation/*.sh' \
    --include='homeAutomation/*.py' \
    --include='homeAutomation/*.conf' \
    --include='homeAutomation/*.sql' \
    --include='homeAutomation/*.yml' \
    --include='homeAutomation/*.yaml' \
    --include='homeAutomation/*.md' \
    --include='homeAutomation/*.txt' \
    --include='homeAutomation/*.nix' \
    --include='homeAutomation/.env.example' \
    --include='homeAutomation/.env-simple' \
    --include='homeAutomation/.envrc' \
    --include='homeAutomation/.gitignore' \
    --include='homeAutomation/Dockerfile' \
    --include='homeAutomation/grafana/' \
    --include='homeAutomation/grafana/provisioning/' \
    --include='homeAutomation/grafana/provisioning/**' \
    --include='homeAutomation/homeassistant/' \
    --include='homeAutomation/homeassistant/*.yaml' \
    --include='homeAutomation/homeassistant/*.template' \
    --include='homeAutomation/homeassistant/blueprints/' \
    --include='homeAutomation/homeassistant/blueprints/**' \
    --include='homeAutomation/timescaledb/' \
    --include='homeAutomation/timescaledb/init/' \
    --include='homeAutomation/timescaledb/init/**' \
    --include='.env.example' \
    --include='README.md' \
    --exclude='*' \
    --exclude='homeAutomation/.env' \
    --exclude='homeAutomation/grafana/data/**' \
    --exclude='homeAutomation/grafana_data/**' \
    --exclude='homeAutomation/mosquitto/data/**' \
    --exclude='homeAutomation/mosquitto/log/**' \
    --exclude='homeAutomation/mosquitto/config/passwd' \
    --exclude='homeAutomation/mosquitto/config/acl' \
    --exclude='homeAutomation/timescaledb/data/**' \
    --exclude='homeAutomation/node-red/**' \
    --exclude='homeAutomation/postgres_data/**' \
    --exclude='homeAutomation/influxdb/data/**' \
    --exclude='homeAutomation/homeassistant/home-assistant_v2.db' \
    --exclude='homeAutomation/homeassistant/home-assistant.log*' \
    --exclude='homeAutomation/homeassistant/known_devices.yaml' \
    --exclude='homeAutomation/homeassistant/secrets.yaml' \
    --exclude='homeAutomation/homeassistant/.storage/**' \
    --exclude='homeAutomation/homeassistant/.cloud/**' \
    --exclude='homeAutomation/homeassistant/deps/**' \
    --exclude='homeAutomation/homeassistant/tts/**' \
    --exclude='homeAutomation/.direnv/**' \
    --exclude='homeAutomation/.idea/**' \
    --exclude='.git/' \
    --exclude='.pio/' \
    --exclude='.vscode/' \
    --exclude='*.bak' \
    --exclude='*.backup' \
    --exclude='.DS_Store' \
    --exclude='Thumbs.db' \
    "$LOCAL_PATH/" "$REMOTE_HOST:$REMOTE_PATH/"

# Check rsync exit code and continue despite warnings
RSYNC_EXIT=$?
if [ $RSYNC_EXIT -eq 0 ]; then
    echo -e "${GREEN}✅ Infrastructure files synchronized successfully${NC}"
elif [ $RSYNC_EXIT -eq 23 ]; then
    echo -e "${YELLOW}⚠️ Infrastructure files synchronized with some warnings (permission issues on excluded files - this is normal)${NC}"
else
    echo -e "${RED}❌ Rsync failed with exit code $RSYNC_EXIT${NC}"
    exit $RSYNC_EXIT
fi

# Check for environment file
echo -e "\n${YELLOW}🔧 Environment configuration...${NC}"
if [[ ! -f "homeAutomation/.env" ]]; then
    echo -e "${YELLOW}⚠️  No .env file found locally${NC}"
else
    echo -e "${YELLOW}📋 .env file found locally but NOT copied (security)${NC}"
fi

# Check if .env exists on remote
if ssh "$REMOTE_HOST" "[ -f $REMOTE_PATH/homeAutomation/.env ]"; then
    echo -e "${GREEN}✅ .env file exists on remote${NC}"
else
    echo -e "${RED}❌ No .env file on remote${NC}"
    echo -e "${YELLOW}   Creating .env from template...${NC}"
    ssh "$REMOTE_HOST" "cd $REMOTE_PATH/homeAutomation && cp .env.example .env"
    echo -e "${YELLOW}📝 Please edit the .env file on remote:${NC}"
    echo -e "${BLUE}   ssh $REMOTE_HOST 'cd $REMOTE_PATH/homeAutomation && nano .env'${NC}"
fi

# Create necessary directories on remote (clean structure)
echo -e "\n${YELLOW}📁 Creating clean data directories on remote...${NC}"
ssh "$REMOTE_HOST" "cd $REMOTE_PATH/homeAutomation && \
    mkdir -p \
    mosquitto/data \
    mosquitto/log \
    mosquitto/config \
    timescaledb/data \
    grafana/data \
    homeassistant \
    node-red \
    postgres_data \
    influxdb/data && \
    \
    # Set proper ownership for Docker containers \
    sudo chown -R 1000:1000 grafana/data 2>/dev/null || chown -R \$(id -u):\$(id -g) grafana/data && \
    sudo chown -R 999:999 postgres_data 2>/dev/null || chown -R \$(id -u):\$(id -g) postgres_data && \
    sudo chown -R 1883:1883 mosquitto/data mosquitto/log 2>/dev/null || chown -R \$(id -u):\$(id -g) mosquitto/data mosquitto/log"

echo -e "${GREEN}✅ Clean data directories created${NC}"

# Set correct permissions
echo -e "\n${YELLOW}🔐 Setting permissions...${NC}"
ssh "$REMOTE_HOST" "cd $REMOTE_PATH/homeAutomation && \
    chmod +x *.sh && \
    chmod 755 mosquitto/config 2>/dev/null || true && \
    chmod 755 timescaledb/init 2>/dev/null || true"

echo -e "${GREEN}✅ Permissions set${NC}"

# Display next steps
echo -e "\n${GREEN}🎉 Infrastructure deployment completed successfully!${NC}"
echo -e "\n${BLUE}📋 Next steps:${NC}"
echo -e "1. ${YELLOW}Configure environment variables:${NC}"
echo -e "   ${BLUE}ssh $REMOTE_HOST 'cd $REMOTE_PATH/homeAutomation && nano .env'${NC}"
echo -e "\n2. ${YELLOW}Start the services:${NC}"
echo -e "   ${BLUE}ssh $REMOTE_HOST 'cd $REMOTE_PATH/homeAutomation && docker-compose up -d'${NC}"
echo -e "\n3. ${YELLOW}Check status:${NC}"
echo -e "   ${BLUE}ssh $REMOTE_HOST 'cd $REMOTE_PATH/homeAutomation && docker-compose ps'${NC}"

echo -e "\n${GREEN}🔗 Access URLs (replace with your remote IP):${NC}"
echo -e "   • Home Assistant: ${BLUE}http://[REMOTE_IP]:8123${NC}"
echo -e "   • Grafana: ${BLUE}http://[REMOTE_IP]:3000${NC}"
echo -e "   • Node-RED: ${BLUE}http://[REMOTE_IP]:1880${NC}"

echo -e "\n${YELLOW}💡 ESP32 Development:${NC}"
echo -e "   • Compile and upload ESP32 code from your development machine"
echo -e "   • ${BLUE}pio run -e ds18b20-mqtt -t upload --upload-port=/dev/ttyACM0${NC}"
echo -e "   • No need to deploy ESP32 code or credentials to the server"