#include <game.hpp>

unsigned char ASCII_to_ATLAS[255];

float debugratio = 116.0/75.0;

int iround(float x)
{
    return round(x);
}

void init_ASCII_to_ATLAS()
{
    for(int i = 0; i < 26; i++)
    {
        ASCII_to_ATLAS[i+'A'] = i;
        ASCII_to_ATLAS[i+'a'] = i+26;
    }

    for(int i = 0; i < 10; i++)
    {
        ASCII_to_ATLAS[i+'0'] = i+52;
    }

    ASCII_to_ATLAS['.']  = 62;
    ASCII_to_ATLAS[',']  = 63;
    ASCII_to_ATLAS['(']  = 64;
    ASCII_to_ATLAS[')']  = 65;
    ASCII_to_ATLAS['[']  = 66;
    ASCII_to_ATLAS[']']  = 67;
    ASCII_to_ATLAS['_']  = 68;
    ASCII_to_ATLAS['-']  = 69;
    ASCII_to_ATLAS['\\'] = 70;
    ASCII_to_ATLAS['/']  = 71;
    ASCII_to_ATLAS['\''] = 72;
    ASCII_to_ATLAS['<']  = 73;
    ASCII_to_ATLAS['>']  = 74;
    ASCII_to_ATLAS['{']  = 75;
    ASCII_to_ATLAS['}']  = 76;
    ASCII_to_ATLAS[' ']  = 77;
}


UI_text::UI_text(
                std::shared_ptr<UI_tile> textfont,
                const std::string &strtext,
                int screenw,
                int screenh,
                float centered_x,
                float centered_y,
                float charsize, 
                Shader *textshdr,
                const std::string &textnameid)
{
    font = textfont;

    for(Uint64 i = 0; i < strtext.size(); i++)
    {
        text.push_back(ASCII_to_ATLAS[(unsigned int)strtext[i]]);
    }
    
    centered_posx = centered_x*screenw;
    centered_posy = centered_y*screenh;

    charwith = charsize*screenh;

    shader = textshdr;
    nameid = textnameid;
}

bool UI_text::render(GPU_Target *screen, Uint64 mod)
{
    bool hl = false;
    bool hl_letter = false;

    float hcharwith = charwith/2.0;

    float cpx  = centered_posx - hcharwith*(float(text.size()) - 1.0);
    float cpx2 = centered_posx + hcharwith*(float(text.size()) - 1.0);

    font->change_size_norm(charwith, charwith);

    if(mod&TEXT_MOD_SHADER_ALL || mod&TEXT_MOD_SHADER_HL)
    {
        shader->activate();
        // int win_const[4] = {round(charwith*text.size()), screen->h, round(cpx), round(cpx2)};
        
        int win_const[4] = {iround(charwith*text.size()), 
                            iround(charwith), 
                            iround(cpx), 
                            iround(centered_posy-hcharwith)};

        GPU_SetUniformiv(5, 4, 1, win_const);
        GPU_SetUniformf(1, timems/1000.0);
    }

    if(mod&TEXT_MOD_SHADER_HL)
    {
        shader->deactivate();
    }

    if(mod&TEXT_MOD_HL_ALL)
    {
        if(mouse.y > centered_posy-hcharwith && 
           mouse.y < centered_posy+hcharwith &&
           mouse.x > cpx -hcharwith &&
           mouse.x < cpx2+hcharwith
           )
           hl = true;
    }


    for(Uint64 i = 0; i < text.size(); i++)
    {
        font->change_atlas_id(text[i]);

        font->change_position_norm(cpx, centered_posy);

        if(mod&TEXT_MOD_HL_PERLETTER && font->is_mouse_over())
        {
            // font->change_size_norm(charwith*1.25, charwith*1.25);
            // font->render(screen);
            // font->change_size_norm(charwith, charwith);

            hl_letter = true;
        }
        
        if(hl || hl_letter)
        {
            if(mod&TEXT_MOD_SHADER_HL)
                shader->activate();

            if(mod&TEXT_MOD_HL_BIGGER)
                font->change_size_norm(charwith*1.25, charwith*1.25*debugratio);

            font->render(screen);
        }

        else
        {
            if(mod&TEXT_MOD_SHADER_HL)
                shader->deactivate();
                
            font->change_size_norm(charwith, charwith*debugratio);
            font->render(screen);
        }


        hl_letter = false;
        cpx += charwith;
    }

    shader->deactivate();

    return hl;
}