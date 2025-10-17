#!/bin/bash

# Git-based deployment script for Home Automation Infrastructure (Infrastructure only)
# Usage: ./deploy-git.sh [user@]hostname [remote_path] [branch]
# Example: ./deploy-git.sh pi@192.168.1.100 /home/pi/home-automation main

set -e

# Configuration
REMOTE_HOST="${1:-pi@192.168.1.100}"
REMOTE_PATH="${2:-/home/pi/home-automation}"
BRANCH="${3:-main}"
REPO_URL="$(git remote get-url origin 2>/dev/null || echo 'NO_REMOTE')"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚀 Git-based infrastructure deployment to ${REMOTE_HOST}${NC}"
echo -e "${BLUE}Repository: ${REPO_URL}${NC}"
echo -e "${BLUE}Branch: ${BRANCH}${NC}"
echo -e "${BLUE}Remote path: ${REMOTE_PATH}${NC}"

# Check if we have a git remote
if [[ "$REPO_URL" == "NO_REMOTE" ]]; then
    echo -e "${RED}❌ No git remote found. Please set up a git remote first.${NC}"
    echo -e "${YELLOW}   Example: git remote add origin https://github.com/username/repo.git${NC}"
    exit 1
fi

# Test connection
echo -e "\n${YELLOW}📡 Testing connection to remote host...${NC}"
if ! ssh -o ConnectTimeout=5 -o BatchMode=yes "$REMOTE_HOST" exit 2>/dev/null; then
    echo -e "${RED}❌ Cannot connect to $REMOTE_HOST${NC}"
    exit 1
fi

# Check if repository exists on remote
echo -e "\n${YELLOW}📁 Checking remote repository...${NC}"
if ssh "$REMOTE_HOST" "[ -d $REMOTE_PATH/.git ]"; then
    echo -e "${GREEN}✅ Repository exists, pulling latest changes...${NC}"
    ssh "$REMOTE_HOST" "cd $REMOTE_PATH && git fetch origin && git reset --hard origin/$BRANCH"
else
    echo -e "${YELLOW}🔄 Cloning repository...${NC}"
    ssh "$REMOTE_HOST" "git clone $REPO_URL $REMOTE_PATH && cd $REMOTE_PATH && git checkout $BRANCH"
fi

# Remove ESP32-related files from remote (since they're not needed for infrastructure)
echo -e "\n${YELLOW}🧹 Cleaning ESP32-related files from remote...${NC}"
ssh "$REMOTE_HOST" "cd $REMOTE_PATH && \
    rm -rf src/ include/ platformio.ini .pio/ promps/ 2>/dev/null || true"

# Setup environment files only for home automation
echo -e "\n${YELLOW}🔧 Setting up configuration files...${NC}"
ssh "$REMOTE_HOST" "cd $REMOTE_PATH/homeAutomation && \
    [[ ! -f .env ]] && cp .env.example .env || echo '.env already exists'"

# Create data directories
echo -e "\n${YELLOW}📁 Creating data directories...${NC}"
ssh "$REMOTE_HOST" "cd $REMOTE_PATH/homeAutomation && mkdir -p \
    mosquitto/data mosquitto/log mosquitto/config \
    timescaledb/data grafana/data homeassistant node-red postgres_data influxdb/data"

# Set permissions
ssh "$REMOTE_HOST" "cd $REMOTE_PATH/homeAutomation && chmod +x setup.sh setup-simple.sh"

echo -e "\n${GREEN}🎉 Infrastructure deployment completed!${NC}"
echo -e "\n${BLUE}📋 Next steps:${NC}"
echo -e "1. ${YELLOW}Configure environment variables:${NC}"
echo -e "   ${BLUE}ssh $REMOTE_HOST 'cd $REMOTE_PATH/homeAutomation && nano .env'${NC}"
echo -e "\n2. ${YELLOW}Start the services:${NC}"
echo -e "   ${BLUE}ssh $REMOTE_HOST 'cd $REMOTE_PATH/homeAutomation && docker-compose up -d'${NC}"

echo -e "\n${YELLOW}💡 ESP32 Development:${NC}"
echo -e "   • Keep ESP32 development on your local machine"
echo -e "   • ${BLUE}pio run -e ds18b20-mqtt -t upload --upload-port=/dev/ttyACM0${NC}"