<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.fdZimbraServerMailDomain.htmlid}">{if !empty($sectionIcon)}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}</label></span></legend>
  <div>
  <table>
    <tr class="
      {if $attributes.fdZimbraServerMailDomain.subattribute}subattribute{/if}
      {if $attributes.fdZimbraServerMailDomain.required}required{/if}
      {if !$attributes.fdZimbraServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdZimbraServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.fdZimbraServerMailDomain.description|escape}" colspan="4">
        {eval var=$attributes.fdZimbraServerMailDomain.input}
      </td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.fdZimbraServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdZimbraServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.zimbraDomain.description|escape}">
        <label for="{$attributes.zimbraDomain.htmlid}">
          {eval var=$attributes.zimbraDomain.label}
        </label>
      </td>
      <td>{eval var=$attributes.zimbraDomain.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.fdZimbraServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdZimbraServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.zimbraDomainCOS.description|escape}">
        <label for="{$attributes.zimbraDomainCOS.htmlid}">
          {eval var=$attributes.zimbraDomainCOS.label}
        </label>
      </td>
      <td>{eval var=$attributes.zimbraDomainCOS.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.fdZimbraServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdZimbraServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.fdZimbraServerMailDomain_buttons.description|escape}" colspan="4">
        {eval var=$attributes.fdZimbraServerMailDomain_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
