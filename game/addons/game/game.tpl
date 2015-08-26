<style type="text/css">
  {literal}
  div.fd_game {
    background:#FFF6BF none repeat scroll 0 0;
    border-color:#FFD324;
    color:#514721;
    border:2px solid #DDDDDD;
    margin:.2em;
    padding-left:1em;
    padding-right:1em;
    padding-top:.5em;
    padding-bottom:.5em;
  }
  div.success {
    background:#E6EFC2 none repeat scroll 0 0;
    border:2px solid #C6D880;
    color:#264409;
    margin:.2em;
    padding-left:1em;
    padding-right:1em;
    padding-top:.5em;
    padding-bottom:.5em;
    font-size:1.5em;
  }
  div.overlay {
    color:#264409;
    background:none;
    border:none;
    position:absolute;
    width:100%;
    opacity:1;
    text-align:center;
    vertical-align:middle;
    height:100%;
    margin:0;
    padding:0;
    font-size:7em;
    margin-top:15%;
  }
  li.done {
    list-style-type: disc;
  }
  li.not_done {
    list-style-type: circle;
  }
  {/literal}
</style>
{if $mission}
  {if $success}
    <div class="success">
      <img src="plugins/game/images/mission_complete.png" style="vertical-align:middle;height:60px;"/>
      Mission complete
    </div>
  {/if}
  <div class="fd_game">
    <p style="font-size:2em;"><img src="plugins/game/images/game_logo.png" style="height:50px;"/>FusionDirectory - The game<img src="plugins/game/images/game_logo.png" style="height:50px;"/></p>
    <h4>Mission {$mission_number} : {$mission.title}</h4>
    <ul>
      {foreach from=$mission.objectives item=obj}
        {if $obj.done}
          <li class="done">
        {else}
          <li class="not_done">
        {/if}
          {$obj.label}
        </li>
      {/foreach}
    </ul>
  </div>
{else}
  <div class="overlay">
      <img src="plugins/game/images/win.png" style="vertical-align:middle;height:200px;"/>
      You won the game
  </div>
{/if}
