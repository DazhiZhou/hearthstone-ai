﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HearthstoneAI
{
    public partial class frmMain : Form
    {
        LogReader log_reader;

        public string HearthstoneInstallationPath
        {
            get { return this.txtHearthstoneInstallationPath.Text; }
        }

        public frmMain()
        {
            InitializeComponent();
        }

        private void btnChangeHearthstoneInstallationPath_Click(object sender, EventArgs e)
        {
            folderBrowserDialog1.ShowNewFolderButton = false;
            folderBrowserDialog1.SelectedPath = txtHearthstoneInstallationPath.Text;
            folderBrowserDialog1.ShowDialog();
            txtHearthstoneInstallationPath.Text = folderBrowserDialog1.SelectedPath;
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            this.log_reader = new LogReader(this);

            timerMainLoop.Enabled = true;
        }

        private void timerMainLoop_Tick(object sender, EventArgs e)
        {
            this.log_added = false;

            this.log_reader.Process();

            if (this.log_added)
            {
                this.listBoxProcessedLogs.SelectedIndex = this.listBoxProcessedLogs.Items.Count - 1;
                this.listBoxProcessedLogs.TopIndex = this.listBoxProcessedLogs.Items.Count - 1;
            }

            this.UpdateBoard();
        }

        enum GameStage
        {
            STAGE_UNKNOWN,
            STAGE_GAME_FLOW,
            STAGE_PLAYER_MULLIGAN,
            STAGE_OPPONENT_MULLIGAN,
            STAGE_PLAYER_CHOICE,
            STAGE_OPPONENT_CHOICE
        }

        private GameStage GetGameStage(GameState game)
        {
            GameState.Entity game_entity;
            if (!game.TryGetGameEntity(out game_entity)) return GameStage.STAGE_UNKNOWN;

            GameState.Entity player_entity;
            if (!game.TryGetPlayerEntity(out player_entity)) return GameStage.STAGE_UNKNOWN;

            GameState.Entity opponent_entity;
            if (!game.TryGetOpponentEntity(out opponent_entity)) return GameStage.STAGE_UNKNOWN;

            if (player_entity.GetTagOrDefault(GameTag.MULLIGAN_STATE, (int)TAG_MULLIGAN.INVALID) == (int)TAG_MULLIGAN.INPUT)
            {
                return GameStage.STAGE_PLAYER_MULLIGAN;
            }

            if (opponent_entity.GetTagOrDefault(GameTag.MULLIGAN_STATE, (int)TAG_MULLIGAN.INVALID) == (int)TAG_MULLIGAN.INPUT)
            {
                return GameStage.STAGE_OPPONENT_MULLIGAN;
            }

            if (!game_entity.HasTag(GameTag.STEP)) return GameStage.STAGE_UNKNOWN;

            TAG_STEP game_entity_step = (TAG_STEP)game_entity.GetTag(GameTag.STEP);
            if (game_entity_step != TAG_STEP.MAIN_ACTION) return GameStage.STAGE_GAME_FLOW;

            bool player_first = false;
            if (player_entity.GetTagOrDefault(GameTag.FIRST_PLAYER, 0) == 1) player_first = true;
            else if (opponent_entity.GetTagOrDefault(GameTag.FIRST_PLAYER, 0) == 1) player_first = false;
            else throw new Exception("parse failed");

            int turn = game_entity.GetTagOrDefault(GameTag.TURN, -1);
            if (turn < 0) return GameStage.STAGE_UNKNOWN;

            if (player_first && (turn % 2 == 1)) return GameStage.STAGE_PLAYER_CHOICE;
            else if (!player_first && (turn % 2 == 0)) return GameStage.STAGE_PLAYER_CHOICE;
            else return GameStage.STAGE_OPPONENT_CHOICE;
        }

        private string GetGameEntityText(GameState game)
        {
            string result = "";

            GameState.Entity game_entity;
            if (!game.TryGetGameEntity(out game_entity)) return "";

            result += "Stage: " + this.GetGameStage(game).ToString() + Environment.NewLine;

            if (game_entity.HasTag(GameTag.STEP))
            {
                TAG_STEP step = (TAG_STEP)game_entity.GetTag(GameTag.STEP);
                result += "[TAG] STEP = " + step.ToString() + Environment.NewLine;
            }
            result += Environment.NewLine;

            result += "All tags: " + Environment.NewLine;
            foreach (var tag in game_entity.Tags)
            {
                result += "[TAG] " + tag.Key.ToString() + " -> " + tag.Value.ToString() + Environment.NewLine;
            }

            return result;
        }

        private string GetHeroClassFromCardId(string card_id)
        {
            switch (card_id)
            {
                case "HERO_01":
                    return "Warrior";
                case "HERO_02":
                    return "Shaman";
                case "HERO_03":
                    return "Rogue";
                case "HERO_04":
                    return "Paladin";
                case "HERO_05":
                case "HERO_05a":
                    return "Hunter";
                case "HERO_06":
                    return "Druid";
                case "HERO_07":
                    return "Warlock";
                case "HERO_08":
                case "HERO_08a":
                    return "Mage";
                case "HERO_09":
                    return "Priest";
            }
            return "(unknown)";
        }

        private string PrintStateText(GameState.Entity entity, GameTag tag)
        {
            int value = entity.GetTagOrDefault(tag, 0);
            if (value <= 0) return "";

            return "[" + tag.ToString() + ": " + value.ToString() + "] ";
        }

        private string GetEntityExtraStateText(GameState game, GameState.Entity entity)
        {
            string result = "";

            result += this.PrintStateText(entity, GameTag.CHARGE);
            result += this.PrintStateText(entity, GameTag.TAUNT);
            result += this.PrintStateText(entity, GameTag.DIVINE_SHIELD);
            result += this.PrintStateText(entity, GameTag.STEALTH);
            result += this.PrintStateText(entity, GameTag.FORGETFUL);
            result += this.PrintStateText(entity, GameTag.FREEZE);
            result += this.PrintStateText(entity, GameTag.FROZEN);
            result += this.PrintStateText(entity, GameTag.POISONOUS);
            result += this.PrintStateText(entity, GameTag.WINDFURY);

            if (result != "") result += Environment.NewLine;

            return result;
        }

        private string GetHeroPowerText(GameState game, GameState.Entity entity)
        {
            string result = "Hero Power: " + Environment.NewLine;

            result += "\tCard = " + entity.CardId + Environment.NewLine;

            int used = entity.GetTagOrDefault(GameTag.EXHAUSTED, 0);
            if (used != 0) result += "\t[USED]" + Environment.NewLine;

            return result;
        }

        private string GetHeroEntityText(GameState game, GameState.Entity entity)
        {
            string result = "";

            result += "Class: " + this.GetHeroClassFromCardId(entity.CardId) + Environment.NewLine;

            int max_hp = entity.GetTagOrDefault(GameTag.HEALTH, -1);
            int damage = entity.GetTagOrDefault(GameTag.DAMAGE, 0);
            int armor = entity.GetTagOrDefault(GameTag.ARMOR, 0);
            if (max_hp > 0)
            {
                int current_hp = max_hp - damage;
                result += "HP: " + current_hp.ToString() + " + " + armor.ToString() + Environment.NewLine;
            }

            int attack = entity.GetTagOrDefault(GameTag.ATK, 0);
            if (attack > 0)
            {
                result += "Attack: " + attack.ToString() + Environment.NewLine;
            }

            int attacked_this_turn = entity.GetTagOrDefault(GameTag.NUM_ATTACKS_THIS_TURN, 0);
            result += "attacked times: " + attacked_this_turn + Environment.NewLine;

            result += this.GetEntityExtraStateText(game, entity);

            GameState.Entity hero_power;
            if (game.TryGetPlayerHeroPowerEntity(entity.Id, out hero_power))
            {
                result += this.GetHeroPowerText(game, hero_power);
            }

            return result;
        }

        private string GetPlayerEntityText(GameState game, GameState.Entity entity)
        {
            string result = "";

            int crystal = entity.GetTagOrDefault(GameTag.RESOURCES, 0);
            int crystal_this_turn = entity.GetTagOrDefault(GameTag.TEMP_RESOURCES, 0);
            int crystal_used = entity.GetTagOrDefault(GameTag.RESOURCES_USED, 0);
            result += "Crystal: " + (crystal - crystal_used + crystal_this_turn).ToString() + " / " + crystal.ToString() + Environment.NewLine;

            int overload_this_turn = entity.GetTagOrDefault(GameTag.OVERLOAD_LOCKED, 0);
            int overload_next_turn = entity.GetTagOrDefault(GameTag.OVERLOAD_OWED, 0);
            result += "Overload: current = " + overload_this_turn.ToString() +
                " next = " + overload_next_turn.ToString() + Environment.NewLine;

            int hero_entity_id = entity.GetTagOrDefault(GameTag.HERO_ENTITY, -1);
            if (game.Entities.ContainsKey(hero_entity_id))
            {
                result += this.GetHeroEntityText(game, game.Entities[hero_entity_id]);
                result += this.GetEnchantmentsText(game, game.Entities[hero_entity_id]);
            }

            return result;
        }

        private string GetPlayerHeroText(GameState game)
        {
            string result = "";
            GameState.Entity player_entity;
            if (!game.TryGetPlayerEntity(out player_entity)) return "";

            result += GetPlayerEntityText(game, player_entity);
            result += this.GetPlayerWeaponText(game);

            return result;
        }

        private string GetOpponentHeroText(GameState game)
        {
            string result = "";
            GameState.Entity player_entity;
            if (!game.TryGetOpponentEntity(out player_entity)) return "";

            result += GetPlayerEntityText(game, player_entity);

            result += this.GetOpponentWeaponText(game);

            return result;
        }

        private string GetHandCardText(GameState game, GameState.Entity entity)
        {
            return "[Card] Card ID = " + entity.CardId + Environment.NewLine;
        }

        private string GetHandText(GameState game, int controller, TAG_ZONE zone)
        {
            string result = "";

            List<GameState.Entity> items = new List<GameState.Entity>();

            foreach (var entity in game.Entities)
            {
                if (entity.Value.GetTagOrDefault(GameTag.CONTROLLER, controller - 1) != controller) continue;

                if (!entity.Value.HasTag(GameTag.ZONE)) continue;
                if (entity.Value.GetTag(GameTag.ZONE) != (int)zone) continue;

                items.Add(entity.Value);
            }

            items.Sort(GameState.ZonePositionSorter);

            foreach (var entry in items)
            {
                result += this.GetHandCardText(game, entry);
            }

            return result;
        }

        private string GetPlayerHandText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetPlayerEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetHandText(game, controller, TAG_ZONE.HAND);
        }

        private string GetOpponentHandText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetOpponentEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetHandText(game, controller, TAG_ZONE.HAND);
        }

        private string GetDeckCardText(GameState game, GameState.Entity entity)
        {
            return "Known Card ID = " + entity.CardId + Environment.NewLine;
        }

        private string GetDeckText(GameState game, int controller, TAG_ZONE zone)
        {
            string result = "";

            int total_cards = 0;
            List<GameState.Entity> known_cards = new List<GameState.Entity>();

            foreach (var entity in game.Entities)
            {
                if (entity.Value.GetTagOrDefault(GameTag.CONTROLLER, controller - 1) != controller) continue;

                if (!entity.Value.HasTag(GameTag.ZONE)) continue;
                if (entity.Value.GetTag(GameTag.ZONE) != (int)zone) continue;

                total_cards++;
                if (entity.Value.CardId != "") known_cards.Add(entity.Value);
            }

            result += "Total cards: " + total_cards.ToString() + Environment.NewLine;

            foreach (var entry in known_cards)
            {
                result += this.GetDeckCardText(game, entry);
            }

            return result;
        }

        private string GetPlayerDeckText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetPlayerEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetDeckText(game, controller, TAG_ZONE.DECK);
        }

        private string GetOpponentDeckText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetOpponentEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetDeckText(game, controller, TAG_ZONE.DECK);
        }

        private string GetEnchantmentsText(GameState game, GameState.Entity target)
        {
            List<GameState.Entity> enchantments = new List<GameState.Entity>();

            foreach (var entity in game.Entities)
            {
                if (!entity.Value.HasTag(GameTag.CARDTYPE)) continue;
                var card_type = (TAG_CARDTYPE)entity.Value.GetTag(GameTag.CARDTYPE);

                if (card_type != TAG_CARDTYPE.ENCHANTMENT) continue;

                if (entity.Value.GetTagOrDefault(GameTag.ATTACHED, target.Id - 1) != target.Id) continue;

                enchantments.Add(entity.Value);
            }

            if (enchantments.Count == 0) return "";

            string result = "[Enchantments]" + Environment.NewLine;
            foreach (var enchant in enchantments)
            {
                result += "   " + enchant.CardId + Environment.NewLine;
            }
            return result;
        }

        private string GetPlayMinionText(GameState game, GameState.Entity entity)
        {
            string result = "[Minion]" + Environment.NewLine;

            result += "Card ID = " + entity.CardId + Environment.NewLine;

            int max_hp = entity.GetTagOrDefault(GameTag.HEALTH, -1);
            int damage = entity.GetTagOrDefault(GameTag.DAMAGE, 0);
            if (max_hp > 0)
            {
                int current_hp = max_hp - damage;
                result += "HP: " + current_hp.ToString() + " / " + max_hp.ToString() + Environment.NewLine;
            }

            int attack = entity.GetTagOrDefault(GameTag.ATK, 0);
            if (attack > 0)
            {
                result += "Attack: " + attack.ToString() + Environment.NewLine;
            }

            int attacked_this_turn = entity.GetTagOrDefault(GameTag.NUM_ATTACKS_THIS_TURN, 0);
            result += "attacked times: " + attacked_this_turn + Environment.NewLine;

            int exhausted = entity.GetTagOrDefault(GameTag.EXHAUSTED, 0);
            result += "exhausted: " + exhausted + Environment.NewLine;

            result += this.GetEntityExtraStateText(game, entity);

            result += this.GetEnchantmentsText(game, entity);

            return result;
        }

        private string GetPlayMinionsText(GameState game, int controller, TAG_ZONE zone)
        {
            string result = "";

            List<GameState.Entity> sorted_minions = new List<GameState.Entity>();

            foreach (var entity in game.Entities)
            {
                if (entity.Value.GetTagOrDefault(GameTag.CONTROLLER, controller - 1) != controller) continue;

                if (!entity.Value.HasTag(GameTag.ZONE)) continue;
                if (entity.Value.GetTag(GameTag.ZONE) != (int)zone) continue;

                if (!entity.Value.HasTag(GameTag.CARDTYPE)) continue;
                var card_type = (TAG_CARDTYPE)entity.Value.GetTag(GameTag.CARDTYPE);

                if (card_type != TAG_CARDTYPE.MINION) continue;

                sorted_minions.Add(entity.Value);
            }

            sorted_minions.Sort(GameState.ZonePositionSorter);

            foreach (var minion in sorted_minions)
            {
                result += this.GetPlayMinionText(game, minion);
                result += Environment.NewLine;
            }

            return result;
        }

        private string GetPlayerMinionsText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetPlayerEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetPlayMinionsText(game, controller, TAG_ZONE.PLAY);
        }

        private string GetOpponentMinionsText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetOpponentEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetPlayMinionsText(game, controller, TAG_ZONE.PLAY);
        }

        private string GetWeaponText(GameState game, int controller, TAG_ZONE zone)
        {
            string result = "";

            foreach (var entity in game.Entities)
            {
                if (entity.Value.GetTagOrDefault(GameTag.CONTROLLER, controller - 1) != controller) continue;

                if (!entity.Value.HasTag(GameTag.ZONE)) continue;
                if (entity.Value.GetTag(GameTag.ZONE) != (int)zone) continue;

                if (!entity.Value.HasTag(GameTag.CARDTYPE)) continue;
                var card_type = (TAG_CARDTYPE)entity.Value.GetTag(GameTag.CARDTYPE);

                if (card_type != TAG_CARDTYPE.WEAPON) continue;

                result += "[WEAPON] " + entity.Value.CardId + Environment.NewLine;
                result += this.GetEnchantmentsText(game, entity.Value);
            }

            return result;
        }

        private string GetPlayerWeaponText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetPlayerEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            string result = "";
            result += GetWeaponText(game, controller, TAG_ZONE.PLAY);
            return result;
        }

        private string GetOpponentWeaponText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetOpponentEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetWeaponText(game, controller, TAG_ZONE.PLAY);
        }

        private string GetSecretText(GameState game, GameState.Entity entity)
        {
            return "[Card] Card ID = " + entity.CardId + Environment.NewLine;
        }

        private string GetSecretsText(GameState game, int controller, TAG_ZONE zone)
        {
            string result = "";

            foreach (var entity in game.Entities)
            {
                if (entity.Value.GetTagOrDefault(GameTag.CONTROLLER, controller - 1) != controller) continue;

                if (!entity.Value.HasTag(GameTag.ZONE)) continue;
                if (entity.Value.GetTag(GameTag.ZONE) != (int)zone) continue;

                result += this.GetSecretText(game, entity.Value);
            }

            return result;
        }

        private string GetPlayerSecretsText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetPlayerEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetSecretsText(game, controller, TAG_ZONE.SECRET);
        }

        private string GetOpponentSecretsText(GameState game)
        {
            GameState.Entity player_entity;
            if (!game.TryGetOpponentEntity(out player_entity)) return "";

            int controller = player_entity.GetTagOrDefault(GameTag.CONTROLLER, -1);
            if (controller < 0) return "";

            return GetSecretsText(game, controller, TAG_ZONE.SECRET);
        }

        private string GetMulliganText(GameState game, GameState.Entity player)
        {
            string result = "";

            if (player.HasTag(GameTag.MULLIGAN_STATE) == false)
            {
                return "State: UNKNOWN" + Environment.NewLine;
            }

            var state = (TAG_MULLIGAN)player.GetTag(GameTag.MULLIGAN_STATE);
            result += "State: " + state.ToString() + Environment.NewLine;
            
            if (state == TAG_MULLIGAN.INPUT)
            {
                var choices = game.EntityChoices.LastOrDefault(e => e.Value.player_entity_id == player.Id);
                if (choices.Value != null)
                {
                    foreach (var choice in choices.Value.choices)
                    {
                        var choice_entity_id = choice.Value;
                        var choice_entity = game.Entities[choice_entity_id];
                        result += "  Card: " + choice_entity.CardId + Environment.NewLine;
                    }
                }
            }

            return result;
        }

        private string GetMulliganText(GameState game)
        {
            GameState.Entity player;
            if (!game.TryGetPlayerEntity(out player)) return "";

            GameState.Entity opponent;
            if (!game.TryGetOpponentEntity(out opponent)) return "";

            string result = "[Player Mulligan]" + Environment.NewLine;
            result += this.GetMulliganText(game, player);
            result += Environment.NewLine;

            result += "[Opponent Mulligan]" + Environment.NewLine;
            result += this.GetMulliganText(game, opponent);
            result += Environment.NewLine;

            return result;
        }

        private void UpdateBoard()
        {
            var game = this.log_reader.GetGameState();

            this.txtGameEntity.Text = this.GetGameEntityText(game);
            this.txtMulligan.Text = this.GetMulliganText(game);

            this.txtPlayerHero.Text = this.GetPlayerHeroText(game);
            this.txtOpponentHero.Text = this.GetOpponentHeroText(game);
            this.txtPlayerSecrets.Text = this.GetPlayerSecretsText(game);
            this.txtOpponentSecrets.Text = this.GetOpponentSecretsText(game);
            this.txtPlayerHand.Text = this.GetPlayerHandText(game);
            this.txtOpponentHand.Text = this.GetOpponentHandText(game);
            this.txtPlayerDeck.Text = this.GetPlayerDeckText(game);
            this.txtOpponentDeck.Text = this.GetOpponentDeckText(game);
            this.txtPlayerMinions.Text = this.GetPlayerMinionsText(game);
            this.txtOpponentMinions.Text = this.GetOpponentMinionsText(game);
        }

        private void frmMain_Load(object sender, EventArgs e)
        {

        }

        private bool log_added;
        public void AddLog(string log)
        {
            this.listBoxProcessedLogs.Items.Add(log);
            this.log_added = true;
        }
    }
}
