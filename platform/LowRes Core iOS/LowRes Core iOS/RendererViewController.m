//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#import "RendererViewController.h"
#import "core.h"

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 128

typedef struct {
    float Position[3];
    float TexCoord[2];
} Vertex;

const Vertex Vertices[] = {
    {{1, -1, 0}, {(float)SCREEN_WIDTH / (float)TEXTURE_WIDTH, (float)SCREEN_HEIGHT / (float)TEXTURE_HEIGHT}},
    {{1, 1, 0}, {(float)SCREEN_WIDTH / (float)TEXTURE_WIDTH, 0}},
    {{-1, 1, 0}, {0, 0}},
    {{-1, -1, 0}, {0, (float)SCREEN_HEIGHT / (float)TEXTURE_HEIGHT}}
};

const GLushort Indices[] = {
    0, 1, 2,
    2, 3, 0
};


@interface RendererViewController () <GLKViewDelegate>
@property (nonatomic) GLKBaseEffect *effect;
@end

@implementation RendererViewController {
    struct Core *_core;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLuint _texName;
    GLubyte *_textureData;
    BOOL _initialized;
}


- (void)viewDidLoad
{
    [super viewDidLoad];
    
    ((GLKView *)self.view).context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    self.effect = [[GLKBaseEffect alloc] init];
    
    _textureData = (GLubyte *)calloc(TEXTURE_WIDTH * TEXTURE_HEIGHT * 3, sizeof(GLubyte));
}

- (void)dealloc
{
    if ([EAGLContext currentContext] == ((GLKView *)self.view).context)
    {
        [EAGLContext setCurrentContext:nil];
    }
    
    if (_initialized)
    {
        glDeleteBuffers(1, &_vertexBuffer);
        glDeleteBuffers(1, &_indexBuffer);
        glDeleteTextures(1, &_texName);
    }
    
    if (_textureData)
    {
        free(_textureData);
    }
}

- (void)setCore:(struct Core *)core
{
    _core = core;
}

- (void)update
{
    if (_core && _textureData)
    {
        if (_core->interpreter.state != StateEnd)
        {
            core_update(_core);
            video_renderScreen(_core, _textureData, 256*3);
            if (_core->interpreter.state == StateEnd)
            {
                printf("Ended at position %d: %s\n", _core->interpreter.pc->sourcePosition, ErrorStrings[_core->interpreter.exitErrorCode]);
            }
        }
    }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    if (!_initialized)
    {
        glGenBuffers(1, &_vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
        
        glGenBuffers(1, &_indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
        
        glGenTextures(1, &_texName);
        glBindTexture(GL_TEXTURE_2D, _texName);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        self.effect.texture2d0.name = _texName;
        self.effect.texture2d1.enabled = GL_FALSE;
        
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glEnableVertexAttribArray(GLKVertexAttribPosition);
        glVertexAttribPointer(GLKVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *) offsetof(Vertex, Position));
        glEnableVertexAttribArray(GLKVertexAttribTexCoord0);
        glVertexAttribPointer(GLKVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *) offsetof(Vertex, TexCoord));
        
        _initialized = YES;
    }
    
    if (_textureData)
    {
        [self.effect prepareToDraw];
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, _textureData);
        glDrawElements(GL_TRIANGLES, sizeof(Indices)/sizeof(Indices[0]), GL_UNSIGNED_SHORT, 0);
    }
}

@end
